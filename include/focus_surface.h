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
#ifndef FOCUS_SURFACE_H
#define FOCUS_SURFACE_H

#include "ratpoly_fit.h"
#include "mtf_profile_sample.h"
#include "distance_scale.h"

class Focus_surface  {
  public:
  
    Focus_surface(vector< Mtf_profile_sample >& data, int order_n, int order_m, Distance_scale& distance_scale, double p2=0, double p98=1) 
    : data(data), order_n(order_n), order_m(order_m), maxy(-1e50), maxx(-1e50), cscale(distance_scale.chart_scale), p2(p2), p98(p98) {
    
        double miny = 1e50;
        for (size_t i=0; i < data.size(); i++) {
            maxx = std::max(fabs(data[i].p.x), maxx);
            maxy = std::max(fabs(data[i].p.y), maxy);
            miny = std::min(fabs(data[i].p.y), miny);
        }
        
        printf("chart extents: y:(%lf, %lf), x:(%lf), strips start at y=%lf mm\n", miny, maxy, maxx, miny*cscale);
        
        // TODO: this loop is a great candidate for OpenMP, but some care must be taken
        // to protect the (few) shared structures (like "peaks")
        
        double min_fit_err = 1e50;
        double min_midy = maxy*cscale;
        VectorXd best_sol;
        vector<Sample> dummy_data;
        Ratpoly_fit best_fit(dummy_data, 5, 5);
        
        vector<Sample> peak_pts;
        // |15 to 110| in steps of 2.5, width=5 ??
        for (int s=-1; s <= 1; s+=2) {
            for (double d=std::max(10.0, 1.2*miny*cscale); d <= 0.95*maxy*cscale; d += 1.0) {
                double midy = s*d;
                
                double mean_x = 0;
                double wsum = 0;
                
                vector<Sample> pts_row;
                for (size_t i=0; i < data.size(); i++) {
                    double dy = midy - data[i].p.y*cscale;
                    if (fabs(dy) < 15 && fabs(data[i].p.y*cscale) > 5) { // at least 5 mm from centre of chart
                        
                        double yw = exp(-dy*dy/(2*3*3)); // sdev of 3 mm in y direction
                        double cw = softclamp(data[i].mtf, p2, p98);
                        pts_row.push_back( Sample(data[i].p.x*cscale, data[i].mtf, yw, 0.1 + 0.9*exp(2*cw)));
                        mean_x += pts_row.back().weight * data[i].p.y * cscale;
                        wsum += pts_row.back().weight;
                    } 
                }
                
                if (pts_row.size() < 3*14) {
                    continue; 
                }
                
                VectorXd sol;
                double lpeak;
                
                mean_x /= wsum;
                
                Ratpoly_fit cf(pts_row, order_n, order_m);
                //Ratpoly_fit cf(pts_row, 5, 2);
                sol = rpfit(cf, true, true); 
                while (cf.order_n > 1 && cf.order_m > 0 && cf.has_poles(sol)) {
                    cf.order_m--;
                    sol = rpfit(cf, true, true);
                    if (cf.has_poles(sol)) {
                        cf.order_n--;
                        sol = rpfit(cf, true, true);
                    }
                }
                if (cf.has_poles(sol)) { 
                    // no solution without poles, give up, skip this sample?
                    printf("Warning: no viable RP fit. Skipping curve centred at y=%lf\n", mean_x);
                    continue;
                }
                
                #if 0
                // Attempt IRLS
                double prev_err = 1e50;
                for (int iter=0; iter < 30; iter++) {
                    double errsum = 0;
                    double wsum = 0;
                    for (size_t i=0; i < pts_row.size(); i++) {
                        double y = cf.rpeval(sol, pts_row[i].x*cf.xsf) / cf.ysf;
                        double e = fabs(y - pts_row[i].y);
                        
                        //pts_row[i].yweight = (0.1 + 0.9*exp(2*pts_row[i].y)) / std::max(0.0001, e);
                        pts_row[i].yweight = 1.0 / std::max(0.0001, e);
                        errsum += e * pts_row[i].yweight;
                        wsum += pts_row[i].yweight;
                        
                    }
                    errsum /= wsum;
                    printf("iter %d err: %lf\n", iter, errsum);
                    VectorXd oldsol = sol;
                    sol = rpfit(cf, true, true);
                    if (iter > 0 && (prev_err - errsum)/prev_err < 0.0001) {
                        printf("bailing out at iter %d\n", iter);
                        if (errsum > prev_err) {
                            printf("reverting to older solution\n");
                            sol = oldsol;
                        }
                        break;
                    }
                    prev_err = errsum;
                }
                #endif
                
                double err = cf.evaluate(sol);
                lpeak = cf.peak(sol); 
                
                double merr = err/double(pts_row.size());
                if (merr < min_fit_err) {
                    printf("min fit err %lf at dist %lf\n", merr, midy);
                    min_fit_err = merr;
                }
                //if (fabs(midy*merr) < min_midy) {
                if (fabs(midy - 40) < fabs(min_midy - 40) ) {
                    printf("new closest point: %lf, eerr=%lf\n", midy, fabs(midy*merr));
                    //min_midy = fabs(midy*merr);
                    min_midy = midy;
                    best_sol = sol;
                    dummy_data = pts_row;
                    best_fit.order_n = cf.order_n;
                    best_fit.order_m = cf.order_m;
                    best_fit.xsf = cf.xsf;
                    best_fit.ysf = cf.ysf;
                }
                
                double pw = 0.1;
                double xw = fabs(midy)/(0.7*maxy*cscale);
                if (xw < 1) {
                    pw = 0.1 + 0.9*(1 - xw*xw*xw)*(1 - xw*xw*xw)*(1 - xw*xw*xw);
                }
                
                peak_pts.push_back( Sample(mean_x/cscale, lpeak/cscale, pw, 1.0/(1e-4 + err)) );
                ridge_peaks.push_back(Point2d(lpeak/cscale, mean_x/cscale));
                
                //fprintf(stderr, "%lf %lf\n", mean_x/cscale, lpeak/cscale);
            }
        }
        
        if (peak_pts.size() < 10) {
            printf("Not enough peak points to construct peak focus curve.\n");
            return;
        }
        
        Ratpoly_fit cf(peak_pts, 2,2);
        cf.base_value = 1;
        cf.pscale = 0;
        VectorXd sol = rpfit(cf, true, true);
        while (cf.order_m > 0 && cf.has_poles(sol)) {
            cf.order_m--;
            printf("reducing order_m to %d\n", cf.order_m);
            sol = rpfit(cf, true, true);
        }
        if (cf.has_poles(sol)) { 
            // no solution without poles, give up, skip this sample?
            printf("Warning: no viable RP fit to fpeaks data\n");
        }
        
        #if 0
        // attempt IRLS step
        vector<double> iweights(peak_pts.size(), 0);
        for (size_t i=0; i < peak_pts.size(); i++) {
            iweights[i] = peak_pts[i].yweight;
        }
        double prev_err = 1e50;
        for (int iter=0; iter < 30; iter++) {
            double errsum = 0;
            double wsum = 0;
            for (size_t i=0; i < peak_pts.size(); i++) {
                double y = cf.rpeval(sol, peak_pts[i].x*cf.xsf) / cf.ysf;
                double e = fabs(y - peak_pts[i].y);
                peak_pts[i].yweight = iweights[i] / std::max(0.0001, e);
                
                double w = peak_pts[i].yweight;
                errsum += e*w;
                wsum += w;
                
            }
            errsum /= wsum;
            printf("iter %d err: %lf\n", iter, errsum);
            VectorXd oldsol = sol;
            sol = rpfit(cf, true, true);
            if (iter > 10 && (prev_err - errsum)/prev_err < 0.0001) {
                printf("bailing out at iter %d\n", iter);
                if (errsum > prev_err) {
                    printf("reverting to older solution\n");
                    sol = oldsol;
                }
                break;
            }
            prev_err = errsum;
        }
        #endif
        
        // now perform some bootstrapping to obtain bounds on the peak focus curve:
        vector<double> mc_pf;
        map<double, vector<double> > mc_curve;
        for (int iters=0; iters < 30; iters++) {
            vector<Sample> sampled_peak_pts;
            for (int j=0; j < peak_pts.size()*0.5; j++) {
                int idx = (int)floor(peak_pts.size()*double(rand())/double(RAND_MAX));
                sampled_peak_pts.push_back(peak_pts[idx]);
            }
            Ratpoly_fit mc_cf(sampled_peak_pts, cf.order_n, cf.order_m);
            mc_cf.base_value = 1;
            mc_cf.pscale = 0;
            VectorXd mc_sol = rpfit(mc_cf, true, true);
            mc_pf.push_back(mc_cf.rpeval(mc_sol, 0)/mc_cf.ysf);
            
            for (double y=-maxy; y < maxy; y += 10) {
                double x = mc_cf.rpeval(mc_sol, y*mc_cf.xsf)/mc_cf.ysf;
                mc_curve[y].push_back(x);
            }
            
        }
        sort(mc_pf.begin(), mc_pf.end());
        for (map<double, vector<double> >::iterator it = mc_curve.begin(); it != mc_curve.end(); it++) {
            sort(it->second.begin(), it->second.end());
            ridge_p05.push_back(Point2d(it->second[0.05*it->second.size()], it->first));
            ridge_p95.push_back(Point2d(it->second[0.95*it->second.size()], it->first));
        }
        
        for (double y=-maxy; y < maxy; y += 1) {
            double x = cf.rpeval(sol, y*cf.xsf)/cf.ysf;
            ridge.push_back(Point2d(x, y));
        }
        
        double x_inter = cf.rpeval(sol, 0)/cf.ysf;
        
        int x_inter_index = lower_bound(mc_pf.begin(), mc_pf.end(), x_inter) - mc_pf.begin();
        printf("x_inter percentile: %.3lf\n", x_inter_index*100 / double(mc_pf.size()));
        printf("x_inter 95%% confidence interval: [%lf, %lf]\n", mc_pf[0.05*mc_pf.size()], mc_pf[0.95*mc_pf.size()]);
        
        distance_scale.estimate_depth(x_inter, focus_peak);
        distance_scale.estimate_depth(mc_pf[0.05*mc_pf.size()], focus_peak_p05);
        distance_scale.estimate_depth(mc_pf[0.95*mc_pf.size()], focus_peak_p95);
        
        printf("focus_pix %lg\n", x_inter);
        
        printf("focus_plane %lg\n", focus_peak);
        printf("fp_interval: [%lf, %lf]\n", focus_peak_p05, focus_peak_p95);
        
        double curve_min = 1e50;
        double curve_max = -1e50;
        for (size_t i=0; i < dummy_data.size(); i++) {
            curve_min = std::min(curve_min, dummy_data[i].x);
            curve_max = std::max(curve_max, dummy_data[i].x);
        }
        FILE* profile = fopen("nprofile.txt", "wt");
        for (double cx=curve_min; cx <= curve_max; cx += 1) {
            double x = best_fit.rpeval(best_sol, cx*best_fit.xsf)/best_fit.ysf;
            fprintf(profile, "%lf %lf\n", cx, x);
        }
        fclose(profile);
    }
    
    VectorXd rpfit(Ratpoly_fit& cf, bool scale=true, bool refine=true) {
        const vector<Sample>& pts_row = cf.get_data();
        
        if (scale) {
            double xsf=0;
            double ysf=0;
            for (size_t i=0; i < pts_row.size(); i++) {
                xsf = std::max(xsf, fabs(pts_row[i].x));
                ysf = std::max(ysf, fabs(pts_row[i].y));
            }
            cf.xsf = xsf = 1.0/xsf;
            cf.ysf = ysf = 1.0/ysf;
        }
        
        int tdim = cf.dimension();
        MatrixXd cov = MatrixXd::Zero(tdim, tdim);
        VectorXd b = VectorXd::Zero(tdim);
        VectorXd a = VectorXd::Zero(tdim);
        
        VectorXd sol;
        
        for (int iter=0; iter < 1; iter++) {
            cov.setZero();
            b.setZero();
            a.setZero();
            
            for (size_t i=0; i < pts_row.size(); i++) {
                const Sample& sp = pts_row[i];
                double w = sp.weight * sp.yweight;
                a[0] = 1*w;
                double prod = sp.x * cf.xsf; // top poly
                for (int j=1; j <= cf.order_n; j++) {
                    a[j] = prod*w;
                    prod *= sp.x * cf.xsf;
                }
                prod = sp.x*cf.xsf; // bottom poly
                for (int j=1; j <= cf.order_m; j++) {
                    a[j+cf.order_n] = prod*w*sp.y*cf.ysf;
                    prod *= sp.x*cf.xsf;
                }
                
                for (int col=0; col < tdim; col++) { 
                    for (int icol=0; icol < tdim; icol++) { // build covariance of design matrix : A'*A
                        cov(col, icol) += a[col]*a[icol];
                    }
                    b[col] += cf.base_value*a[col]*sp.y*cf.ysf*w; // build rhs of system : A'*b
                }
            }
            
            for (int col=cf.order_n+1; col < cov.cols(); col++) {
                cov.col(col) = -cov.col(col);
            }
            
            sol = cov.jacobiSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(b);
        }
        
        // now perform non-linear optimization
        
        if (refine) {
            sol = cf.gauss_newton_armijo(sol);
        }
        
        return sol;
    }

    double evaluate(VectorXd&) {
        return 0;
    }
    
    int dimension(void) {
        return (order_n+1 + order_m);
    }
    
    double softclamp(double x, double lower, double upper, double p=0.98) {
        double s = (x - lower) / (upper - lower);
        if (s > p) {
            return 1.0/(1.0 + exp(-3.89182*s));
        }
        return s < 0 ? 0 : s;
    }
    
    vector< Mtf_profile_sample >& data;
    int order_n;
    int order_m;
    double maxy;
    double maxx;
    double cscale;
    vector<Point2d> ridge;
    vector<Point2d> ridge_peaks;
    vector<Point2d> ridge_p05;
    vector<Point2d> ridge_p95;
    double focus_peak;
    double focus_peak_p05;
    double focus_peak_p95;
    
    double p2;
    double p98;
};

#endif
