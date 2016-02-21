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
#ifndef EDGE_RECORD_H
#define EDGE_RECORD_H

#include "common_types.h"

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}


class Edge_record {
  public:
    Edge_record(void) : pooled(false) {
    }

    static void pool_edges(Edge_record& a, Edge_record& b) {
        Edge_record m;
        m.points.resize(a.points.size() + b.points.size());
        m.weights.resize(a.weights.size() + b.weights.size());
        
        for (size_t i=0; i < a.points.size(); i++) {
            m.points[i].first = a.points[i].first - a.centroid.x;
            m.points[i].second = a.points[i].second - a.centroid.y;
            m.weights[i] = a.weights[i];
        }
        size_t as = a.points.size();
        for (size_t i=0; i < b.points.size(); i++) {
            m.points[i+as].first = b.points[i].first - b.centroid.x;
            m.points[i+as].second = b.points[i].second - b.centroid.y;
            m.weights[i+as] = b.weights[i];
        }
        
        m.compute_eigenvector_angle();
        a.angle = m.angle;
        b.angle = m.angle;
        a.pooled = true;
        b.pooled = true;
    }

    bool compatible(const Edge_record& b) {
        double Z;
        
        if (fabs(slope) > 2) {
            Z = (1.0/slope - 1.0/b.slope)/sqrt(sB*sB + b.sB*b.sB);
        } else {
            Z = (slope - b.slope)/sqrt(sB*sB + b.sB*b.sB);
        }

        return fabs(Z) < 1.66; // ~90% confidence interval on t-distribution with ~80 dof
    }
    
    inline double relative_orientation(const Edge_record& b) {
        Point d1(cos(angle), sin(angle));
        Point d2(cos(b.angle), sin(b.angle));
        
        return fabs(d1.x*d2.x + d1.y*d2.y);
    }
    
    void add_point(double x, double y, double gx, double gy) {
        points.push_back(make_pair(x, y));
        double mag = (gx*gx + gy*gy);
        weights.push_back(mag);
    }
    
    pair<double, double> compute_eigenvector_angle(void) {
        double covxx = 0;
        double covxy = 0;
        double covyy = 0;
        
        centroid.x = 0;
        centroid.y = 0;
        wsum = 0;
        for (size_t i=0; i < points.size(); i++) {
            double w = weights[i];
            
            if (w > 0) {
                double temp = w + wsum;
                double delta_x = points[i].first - centroid.x;
                double delta_y = points[i].second - centroid.y;
                double rx = delta_x * w / temp;
                double ry = delta_y * w / temp;
                centroid.x += rx;
                centroid.y += ry;
                
                covxx += wsum * delta_x * rx;
                covyy += wsum * delta_y * ry;
                covxy += wsum * delta_x * ry;
                
                wsum = temp;
            }
        }
        
        covxx /= wsum;
        covxy /= wsum;
        covyy /= wsum;
        
        
        // char. poly: l^2 - (a+d)l + (ad-bc) = 0
        // thus l^2 -(tr)l + det = 0
        double tr = covxx + covyy;
        double det = covxx*covyy - covxy*covxy;
        
        double pa=1.0;
        double pb=-tr;
        double pc=det;
        
        double q = -0.5 * (pb + sgn(pb)*sqrt(pb*pb - 4*pa*pc) );
        double l1 = q/pa;
        double l2 = pc / q;
        
        double l = std::max(l1,l2);
        assert(l >= 0);
        
        double ev[2];
        if (fabs(covxy) > 1e-10) {
            ev[0] = l - covyy;
            ev[1] = covxy;
            slope = ev[0] / ev[1]; // TODO: check this?
        } else {
            printf("Warning: edge is perfectly horizontal / vertical\n");
            ev[0] = 0;
            ev[1] = 1;
            slope = 0;
        }
        
        angle = atan2(-ev[0], ev[1]);
        
        return make_pair(
            sqrt(std::max(fabs(l1), fabs(l2))),
            sqrt(std::min(fabs(l1), fabs(l2)))
        );
    }

    bool reduce(void) { // compute orientation, and remove weak points
        if (weights.size() < 20) { // not enough points to really extract an edge here
            angle = 0;
            rsq = 1.0;
            return false;
        }
        
        renormalize_weights();
        vector<double> inweights(weights);
        pair<double,double> dims = compute_eigenvector_angle();
        Point dir(cos(angle), sin(angle));
        
        double lwidth = dims.second;
        for (size_t i=0; i < points.size(); i++) {
            double dx = points[i].first - centroid.x;
            double dy = points[i].second - centroid.y;
            
            double dot = dx * dir.x + dy * dir.y;
            
            double gw = 1.0;
            double dw = 2*2; 
            
            if (dims.first < 10) {
                dw = 4*2;
                double dotp = dx * (-dir.y) + dy*dir.x;
                if (fabs(dotp) > dims.first) {
                    gw *= 0.1;
                }
            }
            
            gw *= exp(-dot*dot/(dw*lwidth*lwidth));
            if (gw < 1e-10) gw = 0;
            
            weights[i] = inweights[i] * gw;
        }
        
        lwidth = compute_eigenvector_angle().second;
        
        dir = Point(cos(angle), sin(angle));
        for (size_t i=0; i < points.size(); i++) {
            double dx = points[i].first - centroid.x;
            double dy = points[i].second - centroid.y;
            
            double dot = dx * dir.x + dy * dir.y;
            
            double gw = exp(-dot*dot/(2*lwidth*lwidth));
            if (gw < 1e-10) gw = 0;
            
            
            double dotp = dx * (-dir.y) + dy*dir.x;
            if (dims.first < 10 && fabs(dotp) > 2*dims.first) {
                gw *= 0.1;
            }
            
            weights[i] = inweights[i] * gw;            
            
        }
        lwidth = compute_eigenvector_angle().second;
        
        dir = Point(cos(angle), sin(angle));
        for (size_t i=0; i < points.size(); i++) {
            double dx = points[i].first - centroid.x;
            double dy = points[i].second - centroid.y;
            
            double dot = dx * dir.x + dy * dir.y;
            
            double gw = exp(-dot*dot/(4*lwidth*lwidth)); // increase width slightly 
            if (gw < 1e-10) gw = 0;
            
            double dotp = dx * (-dir.y) + dy*dir.x;
            if (dims.first < 10 && fabs(dotp) > 2*dims.first) {
                gw *= 0.1;
            }
            
            weights[i] = inweights[i] * gw;            
            
        }
        pair<double, double> radii =  compute_eigenvector_angle();
        
        sB = radii.second / (radii.first*sqrt(wsum));
        
        rsq = 0.0;  // TODO: what is the appropriate measure of uncertainty in the angle estimate?
        
        return true;
    }

    inline bool is_pooled(void) {
        return pooled;
    }

    double slope;
    double angle;
    double rsq;
    Point  centroid;

  private:
  
    void renormalize_weights(void) {
        double maxw = 0;
        for (size_t i=0; i < weights.size(); i++) {
            maxw = std::max(weights[i], maxw);
        }
        if (maxw > 0) {
            for (size_t i=0; i < weights.size(); i++) {
                weights[i] /= maxw;
            }
        }
    }

    vector< pair<double, double> > points;
    vector< double > weights;
    
    double wsum;

    double mse;
    double sB; // standard error in slope estimate

    double dx;
    double dy;

    bool pooled;
};


#endif 


