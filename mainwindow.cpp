#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include <QProgressBar>
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>
#include <zbar.h>
#include <boost/algorithm/string.hpp>
#include <dirent.h>
#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>
#include <boost/lambda/bind.hpp>
#undef BOOST_NO_CXX11_SCOPED_ENUMS

using namespace cv;
using namespace std;
using namespace zbar;

//std::string out_put_path = "/home/pouya/oo/";
std::string out_put_path = "/media/pouya/My Book/Processd/Orginal/";

//std::string out_put_path_omitted = "/home/pouya/oo/Omitted/";
std::string out_put_path_omitted = "/media/pouya/My Book/Processd/Omitted/";
//std::string out_put_path_omitted_pages = "/home/pouya/oo/Removed/";
std::string out_put_path_omitted_pages = "/media/pouya/My Book/Processd/Removed/";
int threshold_empty_low= 3000;
int threshold_empty_high= 50000;

std::vector<std::vector <int>> ques_page ;



ImageScanner scanner;
Mat element = getStructuringElement( MORPH_ELLIPSE,
                                     Size( 5, 5 ));
int detect_circle(Mat img) ;
std::string read_bar(Mat img);

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    std::vector <int> temp ;
    temp.push_back(0);
    ques_page.push_back(temp); //0
    temp.clear();
    temp.push_back(0);temp.push_back(2); temp.push_back(1); temp.push_back(1);temp.push_back(2); temp.push_back(1); temp.push_back(1); temp.push_back(1) ; temp.push_back(2);
    ques_page.push_back(temp); //1
    temp.clear();
    temp.push_back(0);temp.push_back(2);
    ques_page.push_back(temp); //2
    ques_page.push_back(temp); //3
    ques_page.push_back(temp); //4
    temp.clear();
    temp.push_back(0);temp.push_back(2);temp.push_back(0);temp.push_back(0);temp.push_back(1);temp.push_back(0);temp.push_back(2);
    ques_page.push_back(temp); //5
    ques_page.push_back(temp); //6
    temp.clear();
    temp.push_back(0);temp.push_back(1); temp.push_back(2); temp.push_back(1);temp.push_back(1);
    ques_page.push_back(temp); //7
    temp.clear();
    temp.push_back(0);temp.push_back(1); temp.push_back(1); temp.push_back(1);
    ques_page.push_back(temp); //8
    temp.clear();
    temp.push_back(0);temp.push_back(1); temp.push_back(1); temp.push_back(1);temp.push_back(1);temp.push_back(1);temp.push_back(1);temp.push_back(1);temp.push_back(1);temp.push_back(1);temp.push_back(1);
    ques_page.push_back(temp); //9
    temp.clear();
    temp.push_back(0);temp.push_back(1); temp.push_back(1); temp.push_back(1);temp.push_back(1); temp.push_back(1); temp.push_back(1);
    ques_page.push_back(temp); //10




    if(!boost::filesystem::exists(out_put_path)) {
        boost::filesystem::create_directory(out_put_path ) ;
    }

    if(!boost::filesystem::exists(out_put_path_omitted)) {
        boost::filesystem::create_directory(out_put_path_omitted ) ;
    }

    if(!boost::filesystem::exists(out_put_path_omitted_pages)) {
        boost::filesystem::create_directory(out_put_path_omitted_pages ) ;
    }



}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{

    if(!boost::filesystem::exists(out_put_path)) {
        boost::filesystem::create_directory(out_put_path ) ;
    }

    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                "/home",
                                                QFileDialog::ShowDirsOnly
                                                | QFileDialog::DontResolveSymlinks);
//    QString dir = "/home/pouya/Develop/test" ;
    int error_counter = 100 ;

    if(dir!="") {
        scanner.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 1);



        std::string candidID = "" ;
        std::string sessionID =  "";
        std::string qusNo =  "" ;
        std::string pageNo ="" ;

        vector<cv::String> fn;
        glob(dir.toStdString() +"/*.jpg", fn, false);

        size_t count = fn.size();
        ui->progressBar->setMaximum((int) count-1);

        for (size_t i=0; i<count; i++){

            Mat img = imread(fn[i], IMREAD_GRAYSCALE);


            if(img.empty())
            {
                cout << "can not open " << fn[i] << endl;

            }
            if(img.cols != 2496 || img.rows != 3520)
            {
                cv::resize(img,img,cv::Size(2496,3520)) ;
//                std::cout << "resize" << std::endl;
            }
//            detect_circle (img);

            std::string qr_code = read_bar(img) ;
            if(qr_code=="") { //error or empty
                cv::Mat thr ;
                cv::threshold(img,thr,100,255,THRESH_BINARY_INV);
                double sum = cv::sum(thr).val[0] ;
                if(sum >500000) {
                    if (qusNo=="0") continue ;
                    error_counter+=1 ;
                    std::cout << sum << "-" << candidID <<std::endl ;
                    std::string out_put_path_std = out_put_path + sessionID + "/" ;
                    if(!boost::filesystem::exists(out_put_path_std)) {
                        boost::filesystem::create_directory(out_put_path_std ) ;
                    }

                    out_put_path_std += "error/"  ;
                    if(!boost::filesystem::exists(out_put_path_std)) {
                        boost::filesystem::create_directory(out_put_path_std ) ;
                    }

                    out_put_path_std += std::to_string(error_counter) ;
                    boost::filesystem::copy_file(fn[i],out_put_path_std+".jpg",boost::filesystem::copy_option::overwrite_if_exists) ;

                }


            }
            else {
                std::vector<std::string> strs;
                boost::split(strs, qr_code, boost::is_any_of("-"));
                candidID = strs[0] ;
                sessionID = strs[1] ;
                qusNo = strs[2] ;
                pageNo = strs[3] ;
                std::string out_put_path_std = out_put_path + sessionID + "/" ;
                if(!boost::filesystem::exists(out_put_path_std)) {
                    boost::filesystem::create_directory(out_put_path_std ) ;
                }

                out_put_path_std += candidID + "/" ;
                if(!boost::filesystem::exists(out_put_path_std)) {
                    boost::filesystem::create_directory(out_put_path_std ) ;
                }
                if (qusNo!="0") {

                    out_put_path_std += qusNo + "/" ;
                    if(!boost::filesystem::exists(out_put_path_std)) {
                        boost::filesystem::create_directory(out_put_path_std ) ;
                    }

                    out_put_path_std += pageNo  ;

                }
                else {
                    out_put_path_std += candidID  ;
                }


                boost::filesystem::copy_file(fn[i],out_put_path_std+".jpg",boost::filesystem::copy_option::overwrite_if_exists) ;

            }
            ui->progressBar->setValue((int) i);
        }
        std::cout << error_counter << endl;

        QMessageBox::information(this,"Result","All Files Procseed");
    }
}


int detect_circle(Mat img)
{



    Mat gray;
    img.copyTo(gray);
//    medianBlur(img, gray, 5);
    vector<Vec3f> circles;

    HoughCircles(gray, circles, CV_HOUGH_GRADIENT, 1,
                 gray.rows/16, // change this value to detect circles with different distances to each other
                 100, 30, 15, 30 // change the last two parameters
                                // (min_radius & max_radius) to detect larger circles
                 );
    for( size_t i = 0; i < circles.size(); i++ )
    {
        Vec3i c = circles[i];
        circle( img, Point(c[0], c[1]), c[2], Scalar(0,0,255), 3, 8);
        circle( img, Point(c[0], c[1]), 2, Scalar(0,255,0), 3, 8);
    }
    std::cout << circles.size() << std::endl ;

    cv::Mat resize ;
    cv::resize(img,resize,cv::Size(600,1000));
        imshow("detected circles", resize);
        waitKey();

    return 0;

}

std::string read_bar(Mat img)
{
    Mat ROI;
    cv::Mat(img(cv::Rect(450,200,400,500))).copyTo(ROI);
    cv::threshold(ROI,ROI,200,255,THRESH_BINARY) ;

    int width = ROI.cols;
    int height = ROI.rows;
    uchar *raw = (uchar *)ROI.data;

    zbar::Image image_bar(width, height, "Y800", raw, width * height);

    int n = scanner.scan(image_bar);
    for(Image::SymbolIterator symbol = image_bar.symbol_begin();
         symbol != image_bar.symbol_end();
         ++symbol) {

         return symbol->get_data() ;
     }
    cv::Mat(img(cv::Rect(200,1600,500,400))).copyTo(ROI);
    cv::threshold(ROI,ROI,200,255,THRESH_BINARY) ;

    width = ROI.cols;
    height = ROI.rows;
    raw = (uchar *)ROI.data;
    // wrap image data
    zbar::Image image_bar2(width, height, "Y800", raw, width * height);

    n = scanner.scan(image_bar2);
    for(Image::SymbolIterator symbol = image_bar2.symbol_begin();
         symbol != image_bar2.symbol_end();
         ++symbol) {

         return symbol->get_data() ;
     }


    return "" ;


}

float is_page_empty(Mat img)
{
    cv::Mat dilation_dst ,bin,ROI ;
    cv::Mat(img(cv::Rect(100,900,2250,2050))).copyTo(ROI);
    cv::threshold(ROI,bin,220,255,THRESH_BINARY_INV) ;

    erode( bin, dilation_dst, element );
    double sum2 = cv::sum(dilation_dst).val[0] ;


//    cv::Mat resized,resizedROI,resized_org ;
//    cv::resize(dilation_dst,resized,cv::Size(0,0),0.4,0.4) ;
//    cv::resize(bin,resizedROI,cv::Size(0,0),0.4,0.4) ;
//    cv::resize(ROI,resized_org,cv::Size(0,0),0.4,0.4) ;
//    cv::hconcat(resized,resizedROI,resized);
//    cv::hconcat(resized,resized_org,resized);
//    cv::putText(resized,std::to_string(sum2),cv::Point(300,300),2,5,cv::Scalar(100),2);
//    if(sum2 < threshold_empty_low) {
//        cv::putText(resized,"Empty",cv::Point(300,500),2,5,cv::Scalar(100),2);

//    }
//    cv::imshow("r",resized) ; cv::waitKey(0);

    return sum2;


//    if(sum2 < threshold_empty) {
//        return true;

//    }

//    return false;

}





void MainWindow::on_pushButton_2_clicked()
{


        int omit_counter = 0 ;

        QString source = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                    "/home",
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
        if(source!="") {
            using namespace boost::filesystem;
            using namespace boost::lambda;



            std::vector<std::string> strs;
            std::string in_path = source.toStdString() ;
            boost::split(strs, in_path, boost::is_any_of("/"));
            std::string sessionID = strs.back() ;

            std::string out_put_path_omitted_pages_session = out_put_path_omitted_pages  + sessionID +"/";
            if(!boost::filesystem::exists(out_put_path_omitted_pages_session)) {
                boost::filesystem::create_directory(out_put_path_omitted_pages_session ) ;
            }
            std::string out_put_path_omitted_pages_session_zero = out_put_path_omitted_pages_session  +  "Zero/";
            if(!boost::filesystem::exists(out_put_path_omitted_pages_session_zero)) {
                boost::filesystem::create_directory(out_put_path_omitted_pages_session_zero ) ;
            }


            path the_path( in_path);
            int cnt = std::count_if(
                    directory_iterator(the_path),
                    directory_iterator(),
                    static_cast<bool(*)(const path&)>(is_regular_file) );
            ui->progressBar->setMaximum((int) cnt-1);


            std::string out_put_path_session= out_put_path_omitted + sessionID + "/" ;
            if(!boost::filesystem::exists(out_put_path_session)) {
                boost::filesystem::create_directory(out_put_path_session ) ;
            }


            DIR *dir;
            struct dirent *ent;
            if ((dir = opendir (source.toStdString().c_str())) != NULL) {
              int counter = 0 ;
              while ((ent = readdir (dir)) != NULL) {
                  counter++ ;
                  ui->progressBar->setValue((int) counter);

                  std::string candidID = ent->d_name ;
                  if(candidID !=".." && candidID !="." ) {


                      std::string  out_put_path_std = out_put_path_session + candidID + "/" ;
                      if(!boost::filesystem::exists(out_put_path_std)) {
                          boost::filesystem::create_directory(out_put_path_std ) ;
                      }


                      std::string path_candid =  in_path + "/" + candidID + "/" ;

                      DIR *dir_sub;
                      struct dirent *ent_sub;
                      if ((dir_sub = opendir (path_candid.c_str())) != NULL) {
                        while ((ent_sub = readdir (dir_sub)) != NULL) {
                            std::string ques_No= ent_sub->d_name ;
                            if(ques_No !=".." && ques_No !="." && ques_No.length()<3) {


                                std::string out_put_path_ques = out_put_path_std + ques_No + "/" ;
                                if(!boost::filesystem::exists(out_put_path_ques)) {
                                    boost::filesystem::create_directory(out_put_path_ques ) ;
                                }


                                int omit_page = ques_page.at(std::stoi (sessionID)).at( std::stoi(ques_No)) ;

                                std::string path_ques = path_candid + ques_No + "/" ;

                                vector<cv::String> fn;
                                glob(path_ques +"*.jpg", fn, false);

                                size_t count = fn.size();



                                for (size_t i=omit_page; i<count; i++){


                                    Mat img = imread(fn[i], IMREAD_GRAYSCALE);
                                    if(img.cols != 2496 || img.rows != 3520)
                                    {
                                        cv::resize(img,img,cv::Size(2496,3520)) ;
                                    }

                                    float empty_result = is_page_empty(img) ;

                                    if(empty_result >= threshold_empty_high) {



                                        std::string out_put_path_file = out_put_path_ques + std::to_string(i+omit_page) + ".jpg" ;

                                        boost::filesystem::copy_file(fn[i],out_put_path_file,boost::filesystem::copy_option::overwrite_if_exists) ;

                                    }
                                    else {
                                        if(empty_result >= threshold_empty_low) {

                                            std::string file_name = out_put_path_omitted_pages_session +candidID +"-" + ques_No +"-" + std::to_string(i+omit_page) + ".jpg" ;
                                            boost::filesystem::copy_file(fn[i],file_name,boost::filesystem::copy_option::overwrite_if_exists) ;
                                        }
                                        else {
                                            std::string file_name = out_put_path_omitted_pages_session_zero +candidID +"-" + ques_No +"-" + std::to_string(i+omit_page) + ".jpg" ;
                                            boost::filesystem::copy_file(fn[i],file_name,boost::filesystem::copy_option::overwrite_if_exists) ;

                                        }
                                        omit_counter++ ;
                                    }

                                }
                            }
                        }
                      }
                      closedir (dir_sub);

                  }
              }
              closedir (dir);
              std::cout << omit_counter << endl;

              QMessageBox::information(this,"Result","All Files Procseed");
            }

        }
}

void MainWindow::on_pushButton_3_clicked()
{
    QString source = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                "/home",
                                                QFileDialog::ShowDirsOnly
                                                | QFileDialog::DontResolveSymlinks);
    if(source!="") {
        vector<cv::String> fn;
        glob(source.toStdString() +"/*.jpg", fn, false);

        size_t count = fn.size();
        ui->progressBar->setMaximum((int) count-1);

        for (size_t i=0; i<count; i++){

            Mat img = imread(fn[i], IMREAD_GRAYSCALE);
            std::cout<< fn[i]<<std::endl ;
            Mat gray_barcode ;
            cv::Mat(img(cv::Rect(280,400,1500,600))).copyTo(gray_barcode);


//            cv::threshold(gray_barcode,gray_barcode,250,255,THRESH_BINARY);

            int width = gray_barcode.cols;
            int height = gray_barcode.rows;
            uchar *raw = (uchar *)gray_barcode.data;
            // wrap image data
            zbar::Image image_bar(width, height, "Y800", raw, width * height);
            // scan the image for barcodes
            int res = scanner.scan(image_bar);
            // extract results
            bool sucess = false ;
            for(Image::SymbolIterator symbol = image_bar.symbol_begin();
                symbol != image_bar.symbol_end();
                ++symbol) {
                vector<Point> vp;
                // do something useful with results
                cout << "decoded " << symbol->get_type_name() << " symbol ="  << symbol->get_data()  << endl;

                std::string file_name ;

//                file_name = out_path_file +"/"+ symbol->get_data() + ".jpg";
//                cv::imwrite(file_name,image) ;
//                sucess=true ;
            }

//            cv::imshow("sa",gray_barcode) ;cv::waitKey(0);exit(0);

        }
    }

}
