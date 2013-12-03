/*
Copyright 2011 Frans van den Bergh. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice, this list of
      conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright notice, this list
      of conditions and the following disclaimer in the documentation and/or other materials
      provided with the distribution.

THIS SOFTWARE IS PROVIDED BY Frans van den Bergh ''AS IS'' AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Frans van den Bergh OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those of the
authors and should not be interpreted as representing official policies, either expressed
or implied, of the Council for Scientific and Industrial Research (CSIR).
*/
#ifndef QUADTREE_H
#define QUADTREE_H

#include "geom.h"
#include "multipolygon_geom.h"

//==============================================================================
class Quadtree : public Multipolygon_geom {
  public:
     // constructor for root of tree
    Quadtree(double cx, double cy, const string& fname)
     : Multipolygon_geom(cx, cy, fname),  
       q_tl(0), q_tr(0), q_bl(0), q_br(0), leaf(false) {
    
        partition_polygons(0);
    }

    Quadtree(const Polygon_geom& bounds) 
    : q_tl(0), q_tr(0), q_bl(0), q_br(0), leaf(false) {
        
        bounding_box = bounds;
        total_vertices = 0;
    }
    
    void print_bounds(int level=0) {
        double maxx = -1e12;
        double maxy = -1e12;
        double minx = 1e12;
        double miny = 1e12;
        
        for (size_t p=0; p < parts.size(); p++) {
            for (size_t v=0; v < parts[p].bases.size(); v++) {
                maxx = std::max(parts[p].bases[v][0], maxx);
                maxy = std::max(parts[p].bases[v][1], maxy);
                minx = std::min(parts[p].bases[v][0], minx);
                miny = std::min(parts[p].bases[v][1], miny);
            }
        }
        
        if (parts.size() > 0) {
            printf("## (%d) bounds deltas: %lf %lf %lf %lf (%d)\n",
                level,
                maxx - bounding_box.bases[1][0],
                maxy - bounding_box.bases[2][1],
                minx - bounding_box.bases[3][0],
                miny - bounding_box.bases[0][1],
                q_tl || q_tr || q_bl || q_br
            );
        } else {
            printf("## (%d) no parts %d\n", level, q_tl || q_tr || q_bl || q_br);
        }
    
        for (size_t i=0; i < bounding_box.bases.size(); i++) {
            fprintf(stderr, "%d %lf %lf\n", level, bounding_box.bases[i][0], bounding_box.bases[i][1]);
        }
        fprintf(stderr, "%d %lf %lf\n\n", level, bounding_box.bases[0][0], bounding_box.bases[0][1]);
        
        if (q_tl) q_tl->print_bounds(level+1);
        if (q_tr) q_tr->print_bounds(level+1);
        if (q_bl) q_bl->print_bounds(level+1);
        if (q_br) q_br->print_bounds(level+1);
    }
    
    void add_poly(const Polygon_geom& p) {
        total_vertices += p.bases.size();
        parts.push_back(p);
    }

    virtual double intersection_area(const Geometry& b, double xoffset = 0, double yoffset = 0, bool skip_bounds=false) const {
        // first, check this level's bounding box
        
        double bounds_area = 1;
        bounds_area = bounding_box.intersection_area(b, xoffset, yoffset);
        
        if (bounds_area < 1e-11) {
            return bounds_area;
        }
          
        // if this is a leaf, check parts, else pass on to
        // children
        double area = 0;
        for (size_t p=0; p < parts.size(); p++) {
            // if the part's bounding box area is much smaller than the
            // current quad's area, the use it, otherwise skip bounds
            // check in this polygon
            area += parts[p].intersection_area(
                b, 
                xoffset, yoffset,
                parts[p].bb_area > 0.5*bounding_box.own_area
            );
        }
        if (q_tl) area += q_tl->intersection_area(b, xoffset, yoffset);
        if (q_tr) area += q_tr->intersection_area(b, xoffset, yoffset);
        if (q_bl) area += q_bl->intersection_area(b, xoffset, yoffset);
        if (q_br) area += q_br->intersection_area(b, xoffset, yoffset);
        return area;
    }
    
    virtual bool is_inside(double x, double y) const {
        for (size_t p=0; p < parts.size(); p++) {
            if (parts[p].is_inside(x, y)) {
                return true;
            }
        }
        
        bool inside = false;
        if (q_tl) inside = q_tl->is_inside(x, y);
        if (!inside && q_tr) inside = q_tr->is_inside(x, y);
        if (!inside && q_bl) inside = q_bl->is_inside(x, y);
        if (!inside && q_br) inside = q_br->is_inside(x, y);
        
        return inside;
    }
    
    void partition_polygons(int level = 0, int cumulative_verts=0) {
        
        vector<double> all_x;
        vector<double> all_y;

        if (true || level == 0) {
            for (size_t p=0; p < parts.size(); p++) {
                for (size_t v=0; v < parts[p].bases.size(); v++) {
                    all_x.push_back(parts[p].bases[v][0]);
                    all_y.push_back(parts[p].bases[v][1]);
                }
            }
        } else {
            for (size_t v=0; v < 4; v++) {
                all_x.push_back(bounding_box.bases[v][0]);
                all_y.push_back(bounding_box.bases[v][1]);
            }
        }

        sort(all_x.begin(), all_x.end());
        sort(all_y.begin(), all_y.end());


        // somewhat verbose, but manually build the four quadrants

        Polygon_geom tl;
        Polygon_geom tr;
        Polygon_geom bl;
        Polygon_geom br;

        size_t last = all_x.size() - 1;

        cv::Vec2d v_min(all_x[0], all_y[0]);
        cv::Vec2d v_mid((all_x[last]+all_x[0])/2.0, (all_y[last]+all_y[0])/2.0 );
        cv::Vec2d v_max(all_x[last], all_y[last]);

        tl.bases[3] = cv::Vec2d(v_min[0], v_min[1]);
        tl.bases[2] = cv::Vec2d(v_min[0], v_mid[1]);
        tl.bases[1] = cv::Vec2d(v_mid[0], v_mid[1]);
        tl.bases[0] = cv::Vec2d(v_mid[0], v_min[1]);
        tl.rebuild();

        tr.bases[3] = cv::Vec2d(v_mid[0], v_min[1]);
        tr.bases[2] = cv::Vec2d(v_mid[0], v_mid[1]);
        tr.bases[1] = cv::Vec2d(v_max[0], v_mid[1]);
        tr.bases[0] = cv::Vec2d(v_max[0], v_min[1]);
        tr.rebuild();

        bl.bases[3] = cv::Vec2d(v_min[0], v_mid[1]);
        bl.bases[2] = cv::Vec2d(v_min[0], v_max[1]);
        bl.bases[1] = cv::Vec2d(v_mid[0], v_max[1]);
        bl.bases[0] = cv::Vec2d(v_mid[0], v_mid[1]);
        bl.rebuild();

        br.bases[3] = cv::Vec2d(v_mid[0], v_mid[1]);
        br.bases[2] = cv::Vec2d(v_mid[0], v_max[1]);
        br.bases[1] = cv::Vec2d(v_max[0], v_max[1]);
        br.bases[0] = cv::Vec2d(v_max[0], v_mid[1]);
        br.rebuild();
        
        int child_verts_sum = cumulative_verts;
        int leaf_verts_sum = cumulative_verts;
        int nchildren = 0;

        for (size_t p=0; p < parts.size(); p++) {
            leaf_verts_sum += parts[p].bases.size();
        
            vector<Polygon_geom> np;
            
            np = tl.intersect_greiner_horman(parts[p]);
            if (np.size() > 0) {
                if (!q_tl) {
                    q_tl = new Quadtree(tl);
                    child_verts_sum += 4; // subtree bounds cost
                    nchildren++;
                }
                for (size_t k=0; k < np.size(); k++) {
                    q_tl->add_poly(np[k]);
                    child_verts_sum += np[k].bases.size();
                }
            }
                
            np = tr.intersect_greiner_horman(parts[p]);
            if (np.size() > 0) {
                if (!q_tr) {
                    q_tr = new Quadtree(tr);
                    child_verts_sum += 4; // subtree bounds cost
                    nchildren++;
                }
                for (size_t k=0; k < np.size(); k++) {
                    q_tr->add_poly(np[k]);
                    child_verts_sum += np[k].bases.size();
                }
            }

            np = bl.intersect_greiner_horman(parts[p]);
            if (np.size() > 0) {
            
                if (!q_bl) {
                    q_bl = new Quadtree(bl);
                    child_verts_sum += 4; // subtree bounds cost
                    nchildren++;
                }
                for (size_t k=0; k < np.size(); k++) {
                    q_bl->add_poly(np[k]);
                    child_verts_sum += np[k].bases.size();
                }
            }

            np = br.intersect_greiner_horman(parts[p]);
            if (np.size() > 0) {
                if (!q_br) {
                    q_br = new Quadtree(br);
                    child_verts_sum += 4; // subtree bounds cost
                    nchildren++;
                }
                for (size_t k=0; k < np.size(); k++) {
                    q_br->add_poly(np[k]);
                    child_verts_sum += np[k].bases.size();
                }
            }
        }
        
        // recompute bounds
        if (q_tl) { q_tl->compute_bounding_box(); }
        if (q_tr) { q_tr->compute_bounding_box(); }
        if (q_bl) { q_bl->compute_bounding_box(); }
        if (q_br) { q_br->compute_bounding_box(); }
        
        int vthresh = 20 + 8*level; 
        const int maxdepth = 6;
                
        if (level+1 < maxdepth) {
            if (q_tl && q_tl->total_vertices > vthresh) q_tl->partition_polygons(level+1, cumulative_verts + 4);            
            if (q_tr && q_tr->total_vertices > vthresh) q_tr->partition_polygons(level+1, cumulative_verts + 4);
            if (q_bl && q_bl->total_vertices > vthresh) q_bl->partition_polygons(level+1, cumulative_verts + 4);
            if (q_br && q_br->total_vertices > vthresh) q_br->partition_polygons(level+1, cumulative_verts + 4);
        }
        
        parts.clear();
    }

    double cx;
    double cy;
    double own_area;
      
    Quadtree* q_tl;
    Quadtree* q_tr;
    Quadtree* q_bl;
    Quadtree* q_br;
    
    bool leaf;
};

#endif // QUADTREE_H

