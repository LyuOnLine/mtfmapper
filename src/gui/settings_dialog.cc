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
#include <QtWidgets> 
#include "settings_dialog.h"
#include "settings_dialog.moc"
#include "nonempty_validator.h"

#include "common.h"

#include <iostream>
using std::cout;
using std::endl;

const QString setting_threshold = "setting_threshold_sauvola";
const QString setting_threshold_default = "0.55";
const QString setting_pixsize = "setting_pixelsize";
const QString setting_pixsize_default = "4.78";
const QString setting_bayer = "setting_bayer";
const QString setting_esf_model = "setting_esf_model";
const QString setting_linear_gamma = "setting_gamma";
const Qt::CheckState setting_linear_gamma_default = Qt::Unchecked;
const QString setting_annotation = "setting_annotation";
const Qt::CheckState setting_annotation_default = Qt::Checked;
const QString setting_profile = "setting_profile";
const Qt::CheckState setting_profile_default = Qt::Checked;
const QString setting_grid = "setting_grid";
const Qt::CheckState setting_grid_default = Qt::Checked;
const QString setting_lensprofile = "setting_lensprofile";
const Qt::CheckState setting_lensprofile_default = Qt::Unchecked;
const QString setting_orientation = "setting_orientation";
const Qt::CheckState setting_orientation_default = Qt::Unchecked;
const QString setting_autocrop = "setting_autocrop";
const Qt::CheckState setting_autocrop_default = Qt::Unchecked;
const QString setting_lpmm = "setting_lpmm";
const Qt::CheckState setting_lpmm_default = Qt::Unchecked;
const QString setting_gnuplot_scaled = "setting_gnuplot_scaled";
const Qt::CheckState setting_gnuplot_scaled_default = Qt::Checked;
const QString setting_zscale = "setting_zscale";
const int setting_zscale_default = 0;
const QString setting_ea_f = "setting_equiangular_f";
const QString setting_ea_f_default = "16.0";
const QString setting_sg_f = "setting_stereographic_f";
const QString setting_sg_f_default = "8.0";
const QString setting_gnuplot = "setting_gnuplot";
const QString setting_exiv = "setting_exiv";
const QString setting_dcraw = "setting_dcraw";
const QString setting_lens = "setting_lens_correction_type";
const QString setting_cache = "image_cache_size";
const double setting_cache_default = 1024;
const QString setting_mtf_contrast = "mtf_contrast";
const double setting_mtf_contrast_default = 50.0;
const QString setting_lp1 = "lp1";
const QString setting_lp1_default = "10";
const QString setting_lp2 = "lp2";
const QString setting_lp2_default = "30";
const QString setting_lp3 = "lp3";
const QString setting_lp3_default = "";
const QString setting_arguments = "arguments";
const QString setting_lensprofile_fixed = "lens_profile_fixed_scale";
const Qt::CheckState setting_lensprofile_fixed_default = Qt::Unchecked;
const QString setting_surface_max_value = "surface_max_value";
const QString setting_surface_max_value_default = "0";
const QString setting_surface_max_flag = "surface_max_flag";
const Qt::CheckState setting_surface_max_flag_default = Qt::Unchecked;
#ifdef _WIN32
static QString setting_gnuplot_default = "gnuplot.exe";
static QString setting_exiv_default = "exiv2.exe";
static QString setting_dcraw_default = "dcraw.exe";
#else
const QString setting_gnuplot_default = "/usr/bin/gnuplot";
const QString setting_exiv_default = "/usr/bin/exiv2";
const QString setting_dcraw_default = "/usr/bin/dcraw";
#endif

Settings_dialog::Settings_dialog(QWidget *parent ATTRIBUTE_UNUSED)
 : settings("mtfmapper", "mtfmapper"), gnuplot_img_width(1024)
{
    arguments_label = new QLabel(tr("Arguments:"), this);
    arguments_label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    arguments_line  = new QLineEdit(this);
    arguments_line->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    
    QFontMetrics fm(QApplication::font());
    int reasonable_width = fm.width("1048576000");
    int adv_width = fm.width("Threshold: ");
    int surf_max_reasonable_width = fm.width("999.50");
    
    QDoubleValidator* dv_thresh = new Nonempty_DoubleValidator(0.001, 1.0, 3, 0.55, this);
    threshold_label = new QLabel(tr("Threshold:"), this);
    threshold_label->setMinimumWidth(adv_width);
    threshold_line  = new QLineEdit(this);
    threshold_line->setValidator(dv_thresh);
    threshold_line->setMaximumWidth(reasonable_width);
    
    QDoubleValidator* dv_pixsize = new Nonempty_DoubleValidator(0.001, 999.0, 3, 4.0, this);
    pixsize_label   = new QLabel(tr("Pixel size:"), this);
    pixsize_label->setMinimumWidth(adv_width);
    pixsize_line    = new QLineEdit(this);
    pixsize_line->setValidator(dv_pixsize);
    pixsize_line->setMaximumWidth(reasonable_width);
    
    QDoubleValidator* dv_cachesize = new Nonempty_DoubleValidator(1, 1024*1024, 1, 1024, this);
    cache_label   = new QLabel(tr("Cache size:"), this);
    cache_label->setMinimumWidth(adv_width);
    cache_line    = new QLineEdit(this);
    cache_line->setValidator(dv_cachesize);
    cache_line->setMaximumWidth(reasonable_width);
    
    QIntValidator* dv_contrast = new Nonempty_IntValidator(1, 90, 50, this);
    contrast_label   = new QLabel(tr("MTF-XX:"), this);
    contrast_label->setMinimumWidth(adv_width);
    contrast_line    = new QLineEdit(this);
    contrast_line->setValidator(dv_contrast);
    contrast_line->setMaximumWidth(reasonable_width);
    
    QIntValidator* dv_lpmm = new QIntValidator(1, 500, this);
    lpmm_label  = new QLabel(tr("Lensprofile lp/mm:"), this);
    lp1_line    = new QLineEdit(this);
    lp1_line->setValidator(dv_lpmm);
    lp1_line->setMaximumWidth(fm.width("50000"));
    lp2_line    = new QLineEdit(this);
    lp2_line->setValidator(dv_lpmm);
    lp2_line->setMaximumWidth(fm.width("50000"));
    lp3_line    = new QLineEdit(this);
    lp3_line->setValidator(dv_lpmm);
    lp3_line->setMaximumWidth(fm.width("50000"));

    gnuplot_label  = new QLabel(tr("gnuplot executable:"), this);
    gnuplot_line   = new QLineEdit(this);
    gnuplot_button = new QPushButton(tr("Browse"), this);

    exiv_label  = new QLabel(tr("exiv2 executable:"), this);
    exiv_line   = new QLineEdit(this);
    exiv_button = new QPushButton(tr("Browse"), this);

    dcraw_label  = new QLabel(tr("dcraw executable:"), this);
    dcraw_line   = new QLineEdit(this);
    dcraw_button = new QPushButton(tr("Browse"), this);

    accept_button = new QPushButton(tr("&Accept"), this);
    accept_button->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    cancel_button = new QPushButton(tr("&Cancel"), this);
    cancel_button->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    
    cb_linear_gamma = new QCheckBox("Linear gamma (8 bit)", this);
    cb_annotation   = new QCheckBox("Annotation", this);
    cb_profile      = new QCheckBox("Profile", this);
    cb_grid         = new QCheckBox("Grid", this);
    cb_lensprofile  = new QCheckBox("Lens profile", this);
    cb_orientation  = new QCheckBox("Chart orientation", this);
    cb_autocrop     = new QCheckBox("Autocrop", this);
    cb_lpmm         = new QCheckBox("Line pairs/mm units", this);
    cb_gnuplot_scaled = new QCheckBox("Scale plots to window", this);
    cb_lensprofile_fixed = new QCheckBox("Lens profile fixed scale", this);
    cb_surface_max = new QCheckBox("3D plot z-axis max value", this);
    
    box_colour = new QComboBox;
    box_colour->addItem("none");
    box_colour->addItem("red");
    box_colour->addItem("green");
    box_colour->addItem("blue");
    
    box_esf_model = new QComboBox;
    box_esf_model->addItem("kernel");
    box_esf_model->addItem("loess");
    
    rb_lens_pw_quad  = new QRadioButton("piecewise-quadratic");
    rb_lens_quad  = new QRadioButton("quadratic");
    rb_lens_none  = new QRadioButton("none");
    rb_lens_radial = new QRadioButton("radial");
    rb_lens_equiangular = new QRadioButton("equiangular");
    rb_lens_stereo = new QRadioButton("stereographic");
    
    QDoubleValidator* dv_f = new Nonempty_DoubleValidator(0.01, 999.0, 2, 16.0, this);
    ea_f_label = new QLabel(tr("focal length"), this);
    ea_f_line = new QLineEdit(this);
    ea_f_line->setMaxLength(5);
    ea_f_line->setText(settings.value(setting_ea_f, setting_ea_f_default).toString());
    ea_f_line->setValidator(dv_f);
    ea_f_line->setMaximumWidth(reasonable_width);
    sg_f_label = new QLabel(tr("focal length"), this);
    sg_f_line = new QLineEdit(this);
    sg_f_line->setMaxLength(5);
    sg_f_line->setText(settings.value(setting_sg_f, setting_sg_f_default).toString());
    sg_f_line->setValidator(dv_f);
    sg_f_line->setMaximumWidth(reasonable_width);
    
    QDoubleValidator* dv_sm = new Nonempty_DoubleValidator(0, 999.0, 2, 80.0, this);
    surface_max_value = new QLineEdit(this);
    surface_max_value->setMaxLength(5);
    surface_max_value->setText(settings.value(setting_surface_max_value, setting_surface_max_value_default).toString());
    surface_max_value->setValidator(dv_sm);
    surface_max_value->setMaximumWidth(surf_max_reasonable_width);
    
    threshold_line->setText(settings.value(setting_threshold, setting_threshold_default).toString());
    pixsize_line->setText(settings.value(setting_pixsize, setting_pixsize_default).toString());
    contrast_line->setText(settings.value(setting_mtf_contrast, setting_mtf_contrast_default).toString());
    cache_line->setText(settings.value(setting_cache, setting_cache_default).toString());
    lp1_line->setText(settings.value(setting_lp1, setting_lp1_default).toString());
    lp2_line->setText(settings.value(setting_lp2, setting_lp2_default).toString());
    lp3_line->setText(settings.value(setting_lp3, setting_lp3_default).toString());
    arguments_line->setText(settings.value(setting_arguments, "").toString());
    cb_linear_gamma->setCheckState(
        (Qt::CheckState)settings.value(setting_linear_gamma, setting_linear_gamma_default).toInt()
    );
    cb_annotation->setCheckState(
        (Qt::CheckState)settings.value(setting_annotation, setting_annotation_default).toInt()
    );
    cb_profile->setCheckState(
        (Qt::CheckState)settings.value(setting_profile, setting_profile_default).toInt()
    );
    cb_grid->setCheckState(
        (Qt::CheckState)settings.value(setting_grid, setting_grid_default).toInt()
    );
    cb_lensprofile->setCheckState(
        (Qt::CheckState)settings.value(setting_lensprofile, setting_lensprofile_default).toInt()
    );
    cb_orientation->setCheckState(
        (Qt::CheckState)settings.value(setting_orientation, setting_orientation_default).toInt()
    );
    cb_autocrop->setCheckState(
        (Qt::CheckState)settings.value(setting_autocrop, setting_autocrop_default).toInt()
    );
    cb_lpmm->setCheckState(
        (Qt::CheckState)settings.value(setting_lpmm, setting_lpmm_default).toInt()
    );
    cb_gnuplot_scaled->setCheckState(
        (Qt::CheckState)settings.value(setting_gnuplot_scaled, setting_gnuplot_scaled_default).toInt()
    );
    cb_lensprofile_fixed->setCheckState(
        (Qt::CheckState)settings.value(setting_lensprofile_fixed, setting_lensprofile_fixed_default).toInt()
    );
    cb_surface_max->setCheckState(
        (Qt::CheckState)settings.value(setting_surface_max_flag, setting_surface_max_flag_default).toInt()
    );
    
    box_colour->setCurrentIndex(settings.value(setting_bayer, 0).toInt());
    box_esf_model->setCurrentIndex(settings.value(setting_esf_model, 0).toInt());
    
    rb_lens_pw_quad->setChecked(false);
    rb_lens_quad->setChecked(false);
    rb_lens_none->setChecked(false);
    rb_lens_radial->setChecked(false);
    rb_lens_equiangular->setChecked(false);
    rb_lens_stereo->setChecked(false);
    
    switch(settings.value(setting_lens, 0).toInt()) {
        case 0: rb_lens_pw_quad->setChecked(true); break;
        case 1: rb_lens_quad->setChecked(true); break;
        case 2: rb_lens_none->setChecked(true); break;
        case 3: rb_lens_radial->setChecked(true); break;
        case 4: rb_lens_equiangular->setChecked(true); break;
        case 5: rb_lens_stereo->setChecked(true); break;
    }

    #ifdef _WIN32
    setting_gnuplot_default = QCoreApplication::applicationDirPath() + QString("/gnuplot/gnuplot.exe");
    setting_exiv_default = QCoreApplication::applicationDirPath() + QString("/exiv2/exiv2.exe");
    setting_dcraw_default = QCoreApplication::applicationDirPath() + QString("/dcraw/dcraw.exe");
    #endif

    gnuplot_line->setText(settings.value(setting_gnuplot, setting_gnuplot_default).toString());
    exiv_line->setText(settings.value(setting_exiv, setting_exiv_default).toString());
    dcraw_line->setText(settings.value(setting_dcraw, setting_dcraw_default).toString());
    surface_max_value->setText(settings.value(setting_surface_max_value, setting_surface_max_value_default).toString());
    
    zscale_label = new QLabel("3D plot z-axis relative scale factor", this);
    zscale_slider = new QSlider(Qt::Horizontal, this);
    zscale_slider->setFocusPolicy(Qt::StrongFocus);
    zscale_slider->setTickPosition(QSlider::TicksAbove);
    zscale_slider->setMinimum(0);
    zscale_slider->setMaximum(20);
    zscale_slider->setTickInterval(5);
    zscale_slider->setSingleStep(1);
    zscale_slider->setValue(settings.value(setting_zscale, setting_zscale_default).toInt());

    QGroupBox* voGroupBox = new QGroupBox(tr("Output types"), this);
    QVBoxLayout *vo_layout = new QVBoxLayout;
    vo_layout->addWidget(cb_annotation);
    vo_layout->addWidget(cb_profile);
    vo_layout->addWidget(cb_grid);
    vo_layout->addWidget(cb_lensprofile);
    vo_layout->addWidget(cb_orientation);
    voGroupBox->setLayout(vo_layout);
    
    QGroupBox* v2GroupBox = new QGroupBox(tr("Flags"), this);
    QVBoxLayout *cb_layout = new QVBoxLayout;
    cb_layout->addWidget(cb_linear_gamma);
    cb_layout->addWidget(cb_autocrop);
    cb_layout->addWidget(cb_lpmm);
    cb_layout->addWidget(cb_gnuplot_scaled);
    v2GroupBox->setLayout(cb_layout);
    
    QGroupBox* bayer_GroupBox = new QGroupBox(tr("ESF construction"), this);
    QGridLayout* rb_layout = new QGridLayout;
    bayer_label = new QLabel("Bayer channel", this);
    rb_layout->addWidget(bayer_label, 0, 0);
    rb_layout->addWidget(box_colour, 0, 1);
    esf_model_label = new QLabel("ESF model", this);
    rb_layout->addWidget(esf_model_label, 1, 0);
    rb_layout->addWidget(box_esf_model, 1, 1);
    bayer_GroupBox->setLayout(rb_layout);

    QGroupBox* v3GroupBox = new QGroupBox(tr("Helpers"), this);
    QGridLayout *helper_layout = new QGridLayout;
    helper_layout->addWidget(gnuplot_label, 0, 0);
    helper_layout->addWidget(gnuplot_line, 1, 0);
    helper_layout->addWidget(gnuplot_button, 1, 1);
    helper_layout->addWidget(exiv_label, 2, 0);
    helper_layout->addWidget(exiv_line, 3, 0);
    helper_layout->addWidget(exiv_button, 3, 1);
    helper_layout->addWidget(dcraw_label, 4, 0);
    helper_layout->addWidget(dcraw_line, 5, 0);
    helper_layout->addWidget(dcraw_button, 5, 1);
    v3GroupBox->setLayout(helper_layout);
    
    QGroupBox* lens = new QGroupBox(tr("Lens distortion correction"), this);
    QGridLayout* lens_layout = new QGridLayout;
    lens_layout->addWidget(rb_lens_pw_quad, 0, 0);
    lens_layout->addWidget(rb_lens_quad, 1, 0);
    lens_layout->addWidget(rb_lens_none, 2, 0);
    lens_layout->addWidget(rb_lens_radial, 3, 0);
    lens_layout->addWidget(rb_lens_equiangular, 4, 0);
    lens_layout->addWidget(rb_lens_stereo, 5, 0);
    lens_layout->addWidget(ea_f_label, 4, 1);
    lens_layout->addWidget(ea_f_line, 4, 2);
    lens_layout->addWidget(sg_f_label, 5, 1);
    lens_layout->addWidget(sg_f_line, 5, 2);
    lens->setLayout(lens_layout);
    

    QGroupBox* advanced = new QGroupBox(tr("Advanced"), this);
    QVBoxLayout* adv_layout = new QVBoxLayout;
    
    QHBoxLayout* r1_layout = new QHBoxLayout;
    r1_layout->addWidget(threshold_label);
    r1_layout->addWidget(threshold_line);
    r1_layout->addStretch(2);
    adv_layout->addLayout(r1_layout);
    
    QHBoxLayout* r2_layout = new QHBoxLayout;
    r2_layout->addWidget(pixsize_label);
    r2_layout->addWidget(pixsize_line);
    r2_layout->addWidget(new QLabel("\xc2\xb5m", this));
    r2_layout->addStretch(2);
    adv_layout->addLayout(r2_layout);
    
    QHBoxLayout* r3_layout = new QHBoxLayout;
    r3_layout->addWidget(contrast_label);
    r3_layout->addWidget(contrast_line);
    r3_layout->addWidget(new QLabel("% contrast", this));
    r3_layout->addStretch(2);
    adv_layout->addLayout(r3_layout);
    
    QHBoxLayout* r4_layout = new QHBoxLayout;
    r4_layout->addWidget(lpmm_label);
    r4_layout->addWidget(lp1_line);
    r4_layout->addWidget(lp2_line);
    r4_layout->addWidget(lp3_line);
    r4_layout->addStretch(2);
    adv_layout->addLayout(r4_layout);
    
    QHBoxLayout* r5_layout = new QHBoxLayout;
    r5_layout->addWidget(cb_lensprofile_fixed);
    adv_layout->addLayout(r5_layout);
    
    QHBoxLayout* r6_layout = new QHBoxLayout;
    r6_layout->addWidget(zscale_label);
    r6_layout->addStretch(2);
    adv_layout->addLayout(r6_layout);
    
    QHBoxLayout* r7_layout = new QHBoxLayout;
    r7_layout->addWidget(zscale_slider);
    adv_layout->addLayout(r7_layout);
    
    surface_max_units = new QLabel(cb_lpmm->checkState() == Qt::Unchecked ? "c/p" : "lp/mm", this);
    QHBoxLayout* r8_layout = new QHBoxLayout;
    r8_layout->addWidget(cb_surface_max);
    r8_layout->addWidget(surface_max_value);
    r8_layout->addWidget(surface_max_units);
    r8_layout->addStretch(2);
    adv_layout->addLayout(r8_layout);
    
    QHBoxLayout* r9_layout = new QHBoxLayout;
    r9_layout->addWidget(cache_label);
    r9_layout->addWidget(cache_line);
    r9_layout->addWidget(new QLabel("MB", this));
    r9_layout->addStretch(2);
    adv_layout->addLayout(r9_layout);
    
    QHBoxLayout* r10_layout = new QHBoxLayout;
    r10_layout->addWidget(arguments_label);
    r10_layout->addWidget(arguments_line);
    adv_layout->addLayout(r10_layout);
    
    advanced->setLayout(adv_layout);

    
    QGridLayout* vlayout = new QGridLayout;
    vlayout->addWidget(voGroupBox, 0, 0, 1, 2);
    vlayout->addWidget(v2GroupBox, 1, 0, 1, 2);
    vlayout->addWidget(bayer_GroupBox, 2, 0, 1, 2);
    vlayout->addWidget(v3GroupBox, 0, 3, 1, 2);
    vlayout->addWidget(lens, 1, 3, 1, 2);
    vlayout->addWidget(advanced, 2, 3, 3, 2);
    vlayout->addWidget(accept_button, 4, 0);
    vlayout->addWidget(cancel_button, 4, 1);
    
    connect(accept_button, SIGNAL(clicked()), this, SLOT( save_and_close() ));
    connect(cancel_button, SIGNAL(clicked()), this, SLOT( close() ));
    connect(gnuplot_button, SIGNAL(clicked()), this, SLOT( browse_for_gnuplot() ));
    connect(exiv_button, SIGNAL(clicked()), this, SLOT( browse_for_exiv() ));
    connect(dcraw_button, SIGNAL(clicked()), this, SLOT( browse_for_dcraw() ));
    connect(rb_lens_equiangular, SIGNAL(clicked()), this, SLOT( equiangular_toggled() ));
    connect(rb_lens_stereo, SIGNAL(clicked()), this, SLOT( stereographic_toggled() ));
    connect(cb_lpmm, SIGNAL(clicked()), this, SLOT( lpmm_toggled() ));
    
    setLayout(vlayout);
    setWindowTitle("Preferences");
}

void Settings_dialog::open() {

    show();
}

void Settings_dialog::send_argument_string(bool focus_mode) {
    QString args = QString("-t %1").arg(threshold_line->text());
    if (cb_linear_gamma->checkState()) {
        args = args + QString(" -l");
    }
    
    if (cb_annotation->checkState()) {
        args = args + QString(" -a");
    }
    
    if (cb_profile->checkState()) {
        args = args + QString(" -p");
    }
    
    if (cb_grid->checkState()) {
        args = args + QString(" -s");
    }
    
    if (cb_lensprofile->checkState()) {
        args = args + QString(" --lensprofile");
    }
    
    if (cb_orientation->checkState()) {
        args = args + QString(" --chart-orientation");
    }
    
    if (cb_autocrop->checkState()) {
        args = args + QString(" --autocrop");
    }

    if (cb_lpmm->checkState()) {
        args = args + QString(" --pixelsize " + pixsize_line->text());
    }
    
    switch(box_colour->currentIndex()) {
    case 0: break;
    case 1: args = args + QString(" --bayer red"); break;
    case 2: args = args + QString(" --bayer green"); break;
    case 3: args = args + QString(" --bayer blue"); break;
    }
    
    switch(box_esf_model->currentIndex()) {
    case 0: args = args + QString(" --esf-model kernel"); break;
    case 1: args = args + QString(" --esf-model loess"); break;
    }
    
    if (rb_lens_pw_quad->isChecked()) {
        args = args + QString(" --esf-sampler piecewise-quadratic");
    }
    if (rb_lens_quad->isChecked()) {
        args = args + QString(" --esf-sampler quadratic");
    }
    if (rb_lens_none->isChecked()) {
        args = args + QString(" --esf-sampler line");
    }
    if (rb_lens_radial->isChecked()) {
        args = args + QString(" --optimize-distortion");
    }
    if (rb_lens_equiangular->isChecked()) {
        args = args + QString(" --equiangular %1").arg(ea_f_line->text());
    }
    if (rb_lens_stereo->isChecked()) {
        args = args + QString(" --stereographic %1").arg(sg_f_line->text());
    }
    
    if (cb_gnuplot_scaled->checkState()) {
        args = args + QString(" --gnuplot-width %1").arg(gnuplot_img_width);
    }
    
    if (cb_lensprofile_fixed->checkState()) {
        args = args + QString(" --lensprofile-fixed-size");
    }
    
    if (cb_surface_max->checkState() && surface_max_value->text().length() > 0) {
        double surface_max = surface_max_value->text().toDouble();
        if (!cb_lpmm->checkState() && surface_max > 1.0) {
            surface_max = 0.0;
        }
        args = args + QString(" --surface-max %1").arg(surface_max);
    }
    
    args = args + QString(" --zscale %1").arg(zscale_slider->value()/20.0);
    
    args = args + QString(" --mtf %1").arg(contrast_line->text());
    
    // only add --lpX arguments if we are in lp/mm mode
    if (cb_lpmm->checkState()) {
        if (lp1_line->text().length() > 0) {
            args = args + QString(" --lp1 %1").arg(lp1_line->text());
        }
        
        if (lp2_line->text().length() > 0) {
            args = args + QString(" --lp2 %1").arg(lp2_line->text());
        }
        
        if (lp3_line->text().length() > 0) {
            args = args + QString(" --lp3 %1").arg(lp3_line->text());
        }
    }
    
    if (!focus_mode) {
        args = args + QString(" -q");
    }
    
    args = args + QString(" %1").arg(arguments_line->text());
    
    emit argument_string(args);
}

void Settings_dialog::save_and_close() {
    check_gnuplot_binary();
    check_exiv2_binary();
    check_mtf_lower();
    settings.setValue(setting_threshold, threshold_line->text());
    settings.setValue(setting_pixsize, pixsize_line->text());
    settings.setValue(setting_mtf_contrast, contrast_line->text());
    settings.setValue(setting_linear_gamma, cb_linear_gamma->checkState());
    settings.setValue(setting_annotation, cb_annotation->checkState());
    settings.setValue(setting_profile, cb_profile->checkState());
    settings.setValue(setting_lpmm, cb_lpmm->checkState());
    settings.setValue(setting_grid, cb_grid->checkState());
    settings.setValue(setting_lensprofile, cb_lensprofile->checkState());
    settings.setValue(setting_orientation, cb_orientation->checkState());
    settings.setValue(setting_autocrop, cb_autocrop->checkState());
    settings.setValue(setting_gnuplot_scaled, cb_gnuplot_scaled->checkState());
    settings.setValue(setting_lensprofile_fixed, cb_lensprofile_fixed->checkState());
    settings.setValue(setting_surface_max_flag, cb_surface_max->checkState());
    settings.setValue(setting_gnuplot, gnuplot_line->text());
    settings.setValue(setting_exiv, exiv_line->text());
    settings.setValue(setting_dcraw, dcraw_line->text());
    settings.setValue(setting_zscale, zscale_slider->value());
    settings.setValue(setting_cache, cache_line->text());
    settings.setValue(setting_lp1, lp1_line->text());
    settings.setValue(setting_lp2, lp2_line->text());
    settings.setValue(setting_lp3, lp3_line->text());
    settings.setValue(setting_arguments, arguments_line->text());
    settings.setValue(setting_bayer, box_colour->currentIndex());
    settings.setValue(setting_esf_model, box_esf_model->currentIndex());
    settings.setValue(setting_surface_max_value, surface_max_value->text());
    
    if (rb_lens_pw_quad->isChecked()) {
        settings.setValue(setting_lens, 0);
    }
    if (rb_lens_quad->isChecked()) {
        settings.setValue(setting_lens, 1);
    }    
    if (rb_lens_none->isChecked()) {
        settings.setValue(setting_lens, 2);
    }
    if (rb_lens_radial->isChecked()) {
        settings.setValue(setting_lens, 3);
    }
    if (rb_lens_equiangular->isChecked()) {
        settings.setValue(setting_lens, 4);
        cb_lpmm->setCheckState(Qt::Checked);
        settings.setValue(setting_lpmm, cb_lpmm->checkState());
    }
    if (rb_lens_stereo->isChecked()) {
        settings.setValue(setting_lens, 5);
        cb_lpmm->setCheckState(Qt::Checked);
        settings.setValue(setting_lpmm, cb_lpmm->checkState());
    }
    
    send_argument_string(false);
    set_cache_size(settings.value(setting_cache, setting_cache_default).toInt());
    settings_saved();
    
    close();
}

void Settings_dialog::browse_for_gnuplot(void) {
    QString gnuplot = QFileDialog::getOpenFileName(
        this,
        "Locate gnuplot binary",
        QString("/usr/bin/gnuplot"),
        QString()
    );

    if (gnuplot != QString()) {
        gnuplot_line->setText(gnuplot);
    }

    check_gnuplot_binary();
}

void Settings_dialog::check_mtf_lower(void) {
    double min_contrast = contrast_line->text().toDouble();
    if (min_contrast < 10) {
        QMessageBox::warning(
            this, 
            QString("Low MTF-XX value warning"), 
            QString(
                "Selecting MTF-XX values below 10 may result in unexpected behaviour (some edges may not be"
                " detected, or results may be very sensitive to noise)."
            )
        );
    }
}

void Settings_dialog::check_gnuplot_binary(void) {
    bool gnuplot_exists = QFile::exists(get_gnuplot_binary());
    if (!gnuplot_exists) {
        QMessageBox::warning(
            this, 
            QString("gnuplot helper"), 
            QString("gnuplot helper executable not found. Please reconfigure.")
        );
    }
}

void Settings_dialog::browse_for_exiv(void) {
    QString exiv = QFileDialog::getOpenFileName(
        this,
        "Locate exiv2 binary",
        QString("/usr/bin/exiv2"),
        QString()
    );

    if (exiv != QString()) {
        exiv_line->setText(exiv);
    }

    check_exiv2_binary();
}

void Settings_dialog::check_exiv2_binary(void) {    
    bool exiv_exists = QFile::exists(get_exiv2_binary());
    if (!exiv_exists) {
        QMessageBox::warning(
            this, 
            QString("Exiv2 helper"), 
            QString("Exiv2 helper executable not found. Please reconfigure.")
        );
    }
}

void Settings_dialog::check_dcraw_binary(void) {
    bool dcraw_exists = QFile::exists(get_dcraw_binary());
    if (!dcraw_exists) {
        QMessageBox::warning(
            this, 
            QString("dcraw helper"), 
            QString("dcraw helper executable not found. Please reconfigure.")
        );
    }
}


void Settings_dialog::browse_for_dcraw(void) {
    QString dcraw = QFileDialog::getOpenFileName(
        this,
        "Locate dcraw binary",
        QString("/usr/bin/dcraw"),
        QString()
    );

    if (dcraw != QString()) {
        dcraw_line->setText(dcraw);
    }

    check_exiv2_binary();
}

void Settings_dialog::set_gnuplot_img_width(int w) {
    gnuplot_img_width = w < 1024 ? 1024 : w;
}

QString Settings_dialog::get_gnuplot_binary(void) const {
    return gnuplot_line->text();
}

QString Settings_dialog::get_exiv2_binary(void) const {
    return exiv_line->text();
}

QString Settings_dialog::get_dcraw_binary(void) const {
    return dcraw_line->text();
}

void Settings_dialog::equiangular_toggled() {
    if (rb_lens_equiangular->isChecked() && cb_lpmm->checkState() == Qt::Unchecked) {
        cb_lpmm->setCheckState(Qt::Checked);
    }
}

void Settings_dialog::stereographic_toggled() {
    if (rb_lens_stereo->isChecked() && cb_lpmm->checkState() == Qt::Unchecked) {
        cb_lpmm->setCheckState(Qt::Checked);
    }
}

void Settings_dialog::lpmm_toggled() {
    double resolution_scale = 1.0;
    if (pixsize_line->text().length() > 0) {
        resolution_scale = 1000.0/pixsize_line->text().toDouble();
    }
    if (cb_lpmm->checkState() == Qt::Unchecked) {
        surface_max_units->setText("c/p");
        char buffer[100];
        sprintf(buffer, "%.3lf", surface_max_value->text().toDouble() / resolution_scale);
        surface_max_value->setText(buffer);
    } else {
        surface_max_units->setText("lp/mm");
        char buffer[100];
        sprintf(buffer, "%.2lf", surface_max_value->text().toDouble() * resolution_scale);
        surface_max_value->setText(buffer);
    }
}

QString Settings_dialog::peek_argument_line(void) const {
    return arguments_line->text();
}

void Settings_dialog::reset_argument_line(void) {
    arguments_line->setText(QString(""));
    settings.setValue(setting_arguments, arguments_line->text());
    send_argument_string(false);
}
