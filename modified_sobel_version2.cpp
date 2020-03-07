 
#include<iostream>
#include<vector>
#include<algorithm>

#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"

using namespace cv;
using namespace std;

int min_size_of_edge=30;

struct pixel
{
    int edge_id;
    int value;
    int x,y;
    bool select_status=false;
};

void remove_non_free_elements_MODIFIED_SOBEL(vector<vector<pixel>> &sobel_binary_map,vector<vector<int>>* result)
{
    vector<bool> free_status(result->size());
    for(int a=0;a<result->size();a++)
    {
        if(sobel_binary_map.at(result->at(a).at(0)).at(result->at(a).at(1)).select_status==true)
        {   free_status.at(a)=false;}
        else
        {   free_status.at(a)=true;}
    }
    for(int a=free_status.size()-1;a>=0;a--)
    {
        if(free_status.at(a)==false)
        {   result->erase(result->begin()+a);}
    }
}

void search_for_neighbour_MODIFIED_SOBEL(pixel &current_pixel,vector<vector<pixel>> &sobel_binary_map,vector<vector<int>> &result)
{
    int new_col_index,new_row_index;
    vector<int> co_ordinates(2);
    int delta_co_ordinates[8][2]={{0,-1},{0,1},{-1,0},{1,0},{-1,-1},{-1,1},{1,1},{1,-1}};
    for(int a=0;a<8;a++)
    {
        new_col_index=current_pixel.x+delta_co_ordinates[a][1];
        new_row_index=current_pixel.y+delta_co_ordinates[a][0];
        if(new_col_index>=0 && new_row_index>=0 && 
           new_row_index<sobel_binary_map.size() && 
           new_col_index<sobel_binary_map.at(new_row_index).size() &&
           sobel_binary_map.at(new_row_index).at(new_col_index).select_status==false &&
           sobel_binary_map.at(new_row_index).at(new_col_index).value==255
           )
        {
            co_ordinates.at(0)=new_row_index;
            co_ordinates.at(1)=new_col_index;
            result.push_back(co_ordinates);
        }
    }
}

void bin_color_mapper(Mat* plot,Mat* new_plot,int min_size_of_edge)
{
    //color mapper
    //creating the sobel_binary_map. Here the plotted img is cleaned up by first creating objs using color mapper and than eliminating small objs.
    vector<vector<pixel>> sobel_binary_map;
    for(int a=0;a<plot->rows;a++)
    {
        vector<pixel> pixel_row;
        pixel_row.clear();
        for(int b=0;b<plot->cols;b++)
        {
            pixel pixel_obj;
            pixel_obj.x=b;
            pixel_obj.y=a;
            pixel_obj.value=(int)plot->at<uchar>(a,b);
            pixel_row.push_back(pixel_obj);
        }
        sobel_binary_map.push_back(pixel_row);
    }
    //color mapping work starts here
    vector<vector<pixel*>> edge_vec;
    edge_vec.clear();
    int edge_id=0;
    for(int a=0;a<sobel_binary_map.size();a++)//rows
    {
        for(int b=0;b<sobel_binary_map.at(a).size();b++)//cols
        {
            if(sobel_binary_map.at(a).at(b).select_status==true || sobel_binary_map.at(a).at(b).value==0)
            {   continue;}
            vector<vector<int>> result_buffer;
            result_buffer.clear();
            vector<vector<int>> result;
            result.clear();
            vector<pixel*> edge_pixel_vec;
            edge_pixel_vec.clear();
            // something for the color distance bla bla.....
            vector<pixel> pixel_vec;
            sobel_binary_map.at(a).at(b).edge_id=edge_id;
            edge_pixel_vec.push_back(&sobel_binary_map.at(a).at(b));
            sobel_binary_map.at(a).at(b).select_status=true;
            search_for_neighbour_MODIFIED_SOBEL(sobel_binary_map.at(a).at(b),sobel_binary_map,result);
            point1:
            if(result_buffer.size()!=0)
            {
                sobel_binary_map.at(result_buffer.at(0).at(0)).at(result_buffer.at(0).at(1)).edge_id=edge_id;
                edge_pixel_vec.push_back(&sobel_binary_map.at(result_buffer.at(0).at(0)).at(result_buffer.at(0).at(1)));
                sobel_binary_map.at(result_buffer.at(0).at(0)).at(result_buffer.at(0).at(1)).select_status=true;
                result.clear();
                search_for_neighbour_MODIFIED_SOBEL(sobel_binary_map.at(result_buffer.at(0).at(0)).at(result_buffer.at(0).at(1)),sobel_binary_map,result);
            }
            while(result.size()>0)
            {
                sobel_binary_map.at(result.at(0).at(0)).at(result.at(0).at(1)).edge_id=edge_id;
                edge_pixel_vec.push_back(&sobel_binary_map.at(result.at(0).at(0)).at(result.at(0).at(1)));
                sobel_binary_map.at(result.at(0).at(0)).at(result.at(0).at(1)).select_status=true;
                int row_index=result.at(0).at(0),col_index=result.at(0).at(1);
                if(result.size()>1)
                {
                    for(int c=1;c<result.size();c++)
                    {   result_buffer.push_back(result.at(c));}
                }
                result.clear();
                search_for_neighbour_MODIFIED_SOBEL(sobel_binary_map.at(row_index).at(col_index),sobel_binary_map,result);
            }
            remove_non_free_elements_MODIFIED_SOBEL(sobel_binary_map,&result_buffer);
            if(result_buffer.size()!=0)
            {   goto point1;}
            edge_id++;
            edge_vec.push_back(edge_pixel_vec);
        }
    }
    //cleaning up small edges
    for(int a=edge_vec.size()-1;a>=0;a--)
    {
        if(edge_vec.at(a).size()<min_size_of_edge)//10
        {
            edge_vec.erase(edge_vec.begin()+a);
        }
    }
    //testing the plotter algorithm
    //Mat* new_plot=new Mat(plot->size(),CV_8U,Scalar(0));
    for(int a=0;a<edge_vec.size();a++)
    {
        for(int b=0;b<edge_vec.at(a).size();b++)
        {
            new_plot->at<uchar>(edge_vec.at(a).at(b)->y,edge_vec.at(a).at(b)->x)=255;
        }
    }
}

void plot_combiner(vector<Mat*>& input_plot_vec,Mat* new_plot)
{
    for(int a=0;a<new_plot->rows;a++)
    {
        for(int b=0;b<new_plot->cols;b++)
        {
            for(int c=0;c<input_plot_vec.size();c++)
            {
                if(((int)input_plot_vec[c]->at<uchar>(a,b))==255)
                {   
                    new_plot->at<uchar>(a,b)=255;
                    break; 
                }
            }
        }
    }
}

void variable_thershold_sobel(Mat &img1,string src_img)
{
    Mat* sobel;
    vector<Mat*> sobel_vec;
    vector<Mat*> img_vec;
    img_vec.push_back(&img1);
    Mat hsv;
    cvtColor(img1,hsv,CV_BGR2HSV);
    vector<Mat> hsv_channels;
    split(hsv,hsv_channels);
    Mat s=hsv_channels[1];  
    img_vec.push_back(&s);
    Mat v=hsv_channels[2]; 
    img_vec.push_back(&v);

    //sobel
    int scale = 1;
    int delta = 0;
    int ddepth = CV_16S;
    for(int a=0;a<img_vec.size();a++)
    {
        Mat *src_gray=new Mat();
        sobel=new Mat();
        GaussianBlur(*img_vec[a],*img_vec[a], Size(3,3), 0, 0, BORDER_DEFAULT );
        if(a==0)
        {   cvtColor(*img_vec[a],*src_gray, COLOR_BGR2GRAY );}
        else
        {   src_gray=img_vec[a];}
        Mat grad_x, grad_y;
        Mat abs_grad_x, abs_grad_y;
        Scharr(*src_gray, grad_x, ddepth, 1, 0, scale, delta, BORDER_DEFAULT );
        Sobel(*src_gray, grad_x, ddepth, 1, 0, 3, scale, delta, BORDER_DEFAULT );//remove these 2 for best result alita
        Scharr(*src_gray, grad_y, ddepth, 0, 1, scale, delta, BORDER_DEFAULT );
        Sobel(*src_gray, grad_y, ddepth, 0, 1, 3, scale, delta, BORDER_DEFAULT );//3
        convertScaleAbs( grad_x, abs_grad_x );
        convertScaleAbs( grad_y, abs_grad_y );
        addWeighted( abs_grad_x, 1, abs_grad_y, 1, 0,*sobel);//0.5
        sobel_vec.push_back(sobel);
        src_gray->release();
        grad_x.release();
        grad_y.release();
        abs_grad_x.release();
        abs_grad_y.release();
        img_vec[a]->release();
    }
    img_vec.clear();
    string str1="sobel_result_"+src_img;
    imwrite(str1,*sobel_vec[0]);
    //imwrite("b.jpg",*sobel_vec[1]);
    //slicing sorting 
    struct sortingclass 
    {
        bool operator() (pixel obj1,pixel obj2) 
        { return obj1.value>obj2.value;}
    }sorting_helper;
    int slice_row_size=40,slice_col_size=40;//40
    vector<Mat*> plot_vec;
    for(int x=0;x<sobel_vec.size();x++)
    {
        vector<pixel> pixels;
        Mat* plot=new Mat(sobel_vec[x]->size(), CV_8U,Scalar(0));
        for(int a=0;a<sobel_vec[x]->rows;a+=slice_row_size)
        {
            for(int b=0;b<sobel_vec[x]->cols;b+=slice_col_size)
            {
                if(b+slice_col_size>=sobel_vec[x]->cols || a+slice_row_size>=sobel_vec[x]->rows)
                {   break;}
                Mat ROI(*sobel_vec[x],Rect(b,a,slice_col_size,slice_row_size));
                for(int c=0;c<slice_row_size;c++)
                {
                    for(int d=0;d<slice_col_size;d++)
                    {   
                        pixel new_pixel;
                        new_pixel.x=c;
                        new_pixel.y=d;
                        new_pixel.value=(int)ROI.at<uchar>(c,d);
                        pixels.push_back(new_pixel);
                    }
                }
                sort(pixels.begin(),pixels.end(),sorting_helper);//sorts in descending order
                int point=0,last=0,count=0;
                for(int c=0;c<slice_col_size*slice_row_size;c++)
                {
                    if(last==pixels[c].value)
                    {   count++;}
                    else
                    {
                        last=pixels[c].value;
                        count=0;
                    }
                    if(count>10 && pixels[c].value<240)//if more than 10 pixels same selete the thershold point
                    {   
                        point=c;
                        break;
                    }
                }
                for(int c=0;c<point;c++)
                {   plot->at<uchar>(a+pixels[c].x,b+pixels[c].y)=255;}
                pixels.clear();
            }
        }
        sobel_vec[x]->release();
        plot_vec.push_back(plot);
    }
    sobel_vec.clear();
    Mat* new_plot1=new Mat(plot_vec[0]->size(),CV_8U,Scalar(0));

    plot_combiner(plot_vec,new_plot1);
    Mat* new_plot2=new Mat(plot_vec[0]->size(),CV_8U,Scalar(0));
    bin_color_mapper(new_plot1,new_plot2,min_size_of_edge);
    src_img="result_"+src_img;
    imwrite(src_img,*new_plot2);
    new_plot2->release();
}

int main(int argc, char** argv)
{
    string src_img="";
    for(int a=0;a<argc;a++)
    {
        string str=argv[a];
    int index=str.find("-src=");
    if(index!=-1)
    {    src_img=str.substr(5,str.length());}
    }
    Mat src=imread(src_img);
    if(src.empty())
    {   cout<<"\nERROR!! Failed to load the image: "<<src_img<<endl<<endl;}
    else
    {
        char option1;
        cout<<"\n";
        cout<<"\nDo you want to continue with the default min_size_of_edge settings, default is 30?(y/n): ";   
        cin>>option1;
        if(option1=='n' || option1== 'N')
        {    
            cout<<"\nenter the new value=";
            cin>>min_size_of_edge;
        }
        float row=1.0/1,col=1.0/1;
        Mat dst;
        resize(src,dst,dst.size(),row,col);//cols,rows or x,y
        variable_thershold_sobel(src,src_img);    
    }
}
