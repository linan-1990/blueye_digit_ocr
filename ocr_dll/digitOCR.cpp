/*
* test digits OCR
*
* @author  Nan Li
* @since   July 2020
*/

#include "stdafx.h"
#include "digitOCR.h"
#include "Resource.h"
#include <GdiPlus.h>


RNG rng(12345);

Mat findConvexPoly(Mat& src)
{
    Mat blurImage, edges;
    blur(src, blurImage, Size(3, 3));
    Canny(blurImage, edges, 50, 100);

    vector<vector<Point>> contours;
    findContours(edges, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
    sort(contours.begin(), contours.end(), [&contours](vector<Point> lhs, vector<Point> rhs) { return lhs.size() > rhs.size(); });

    vector<Point> hull;
    convexHull(Mat(contours[0]), hull, false);

    vector<vector<Point>> cnt(1);
    approxPolyDP(hull, cnt[0], 20, true);

    Mat poly = Mat::zeros(edges.size(), CV_8UC1);
    drawContours(poly, cnt, 0, Scalar(255));

    return poly;
}

vector<Point2f> detectCornors(Mat& src)
{
    vector<Vec2f> lines;
    HoughLines(src, lines, 1, CV_PI / 180, 50, 0, 0);

    Mat labels, centers;
    vector<Point2f> data;
    for (size_t i = 0; i < lines.size(); i++)
    {
        float rho = lines[i][0], theta = lines[i][1];
        float x = rho * cos(theta), y = rho * sin(theta);
        data.push_back(Point2f(x, y));
    }

    kmeans(data, 4, labels, TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 10, 1.0), 5, KMEANS_PP_CENTERS, centers);

    vector<Point2f> fourPoints, xyPoints;
    for (size_t i = 0; i < 4; i++)
    {
        float x = centers.at<float>(i, 0);
        float y = centers.at<float>(i, 1);
        float rho = sqrt(x * x + y * y);
        float theta = atan2(y, x);

        xyPoints.push_back(Point2f(x, y));
        fourPoints.push_back(Point2f(rho, theta));
    }

    sort(xyPoints.begin(), xyPoints.end(), [](Point2f& lhs, Point2f& rhs) { return abs(lhs.y / lhs.x) < abs(rhs.y / rhs.x); });

    vector<Point2f> ans;
    for (size_t i = 0; i < 2; i++)
    {
        float x0 = xyPoints[i].x;
        float y0 = xyPoints[i].y;

        for (size_t j = 2; j < 4; j++)
        {
            float x1 = xyPoints[j].x;
            float y1 = xyPoints[j].y;
            float x = (y0 * (x1 * x1 + y1 * y1) - y1 * (x0 * x0 + y0 * y0)) / (y0 * x1 - x0 * y1);
            float y = (x0 * (x1 * x1 + y1 * y1) - x1 * (x0 * x0 + y0 * y0)) / (y1 * x0 - x1 * y0);
            ans.push_back(Point2f(x, y));
        }
    }

    // order of points (top-left, bottom-left, top-right, bottom-right)
    sort(ans.begin(), ans.end(), [](Point2f lhs, Point2f rhs) { return lhs.x < rhs.x; });
    sort(ans.begin(), ans.begin() + 2, [](Point2f lhs, Point2f rhs) { return lhs.y < rhs.y; });
    sort(ans.begin() + 2, ans.end(), [](Point2f lhs, Point2f rhs) { return lhs.y < rhs.y; });

    return ans;
}

Mat LoadFromIDResource(UINT nID)
{
    Mat mat;
    HINSTANCE hInst = AfxGetResourceHandle();
    HRSRC hRsrc = ::FindResource(hInst, MAKEINTRESOURCE(nID), _T("PNG")); // type
    if (!hRsrc)
        return mat;
    // load resource into memory
    DWORD len = SizeofResource(hInst, hRsrc);
    BYTE* lpRsrc = (BYTE*)LoadResource(hInst, hRsrc);
    if (!lpRsrc)
        return mat;
    // Allocate global memory on which to create stream
    HGLOBAL m_hMem = GlobalAlloc(GMEM_FIXED, len);
    BYTE* pmem = (BYTE*)GlobalLock(m_hMem);
    memcpy(pmem, lpRsrc, len);

    //convert to cv::Mat
    std::vector<uchar> data = std::vector<uchar>(pmem, pmem + len);
    mat = imdecode(data, 1);
    //imwrite("image.jpg", mat);
    
    // free/release stuff
    GlobalUnlock(m_hMem);
    FreeResource(lpRsrc);
    return mat;
}

vector<Mat> getRefOCR()
{
    Mat ref = LoadFromIDResource(IDB_FONT);
    cvtColor(ref, ref, COLOR_BGR2GRAY);

    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    findContours(ref, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    /*
    Mat drawing = Mat::zeros(ref.size(), CV_8UC3);
    for (size_t i = 0; i < contours.size(); i++)
    {
        Scalar color = Scalar(rng.uniform(0, 256), rng.uniform(0, 256), rng.uniform(0, 256));
        drawContours(drawing, contours, (int)i, color, 2, LINE_8, hierarchy, 0);
    }
    imshow("Contours", drawing);*/

    sort(contours.begin(), contours.end(), [&contours](vector<Point> lhs, vector<Point> rhs) { return lhs[0].x < rhs[0].x; });

    vector<Mat> roi(contours.size());
    for (size_t i = 0; i < contours.size(); i++)
    {
        Rect boundRect = boundingRect(contours[i]);
        resize(ref(boundRect), roi[i], Size(100, 150));
    }
    return roi;
}

vector<vector<int>> detectDigits(Mat src, Mat gray, vector<Mat> digitROI)
{
    vector<Rect> rects;
    rects.push_back(Rect(0, 0, gray.cols, gray.rows));

    vector<Mat> roi(1);
    vector<vector<int>> results;
    vector<int> ans;
    for (size_t i = 0; i < rects.size(); i++)
    {
        roi[i] = gray(Rect(rects[i].x, rects[i].y, rects[i].width, rects[i].height));
        blur(roi[i], roi[i], Size(3, 3));
        threshold(roi[i], roi[i], 0, 255, THRESH_BINARY | THRESH_OTSU);
        vector<vector<vector<Point>>> digitCnts;
        vector<vector<Point>> digitCnt;
        findContours(roi[i].clone(), digitCnt, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
        if (digitCnt.size() < 6)
        {
            digitCnt.clear();
            threshold(roi[i], roi[i], 0, 255, THRESH_BINARY_INV | THRESH_OTSU);
            findContours(roi[i].clone(), digitCnt, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
        }
        sort(digitCnt.begin(), digitCnt.end(), [](vector<Point> lhs, vector<Point> rhs) { return lhs[0].y < rhs[0].y; });

        // delete noise contours and distribute numbers into different groups based on rows
        double sum_num_height = 0.0;
        for (size_t j = 0; j < digitCnt.size(); j++)
        {
            Rect r = boundingRect(digitCnt[j]);
            sum_num_height += r.height;
        }
        double avg_num_height = sum_num_height / digitCnt.size();
        vector<vector<Point>> temp_contour;
        int last_y = -1;
        for (size_t j = 0; j < digitCnt.size(); j++)
        {
            Rect r = boundingRect(digitCnt[j]);
            if (r.height > 0.5 * avg_num_height && (double)r.height / r.width > 1.2)
            {
                if (last_y == -1)
                    temp_contour.push_back(digitCnt[j]);
                else
                {
                    if (r.y - last_y > avg_num_height)
                    {
                        sort(temp_contour.begin(), temp_contour.end(), [](vector<Point> lhs, vector<Point> rhs) { return lhs[0].x < rhs[0].x; });
                        digitCnts.push_back(temp_contour);
                        temp_contour.clear();
                        temp_contour.push_back(digitCnt[j]);
                    }
                    else
                    {
                        temp_contour.push_back(digitCnt[j]);
                    }
                }
                last_y = r.y;
            }
        }
        // save the last row
        sort(temp_contour.begin(), temp_contour.end(), [](vector<Point> lhs, vector<Point> rhs) { return lhs[0].x < rhs[0].x; });
        digitCnts.push_back(temp_contour);
        temp_contour.clear();
        digitCnt.clear();

        for (size_t ii = 0; ii < digitCnts.size(); ii++)
        {
            for (size_t j = 0; j < digitCnts[ii].size(); j++)
            {
                Rect r = boundingRect(digitCnts[ii][j]);

                Scalar color = Scalar(255, 255, 0);
                rectangle(src, r.tl(), r.br(), color, 1);

                Mat d = roi[i](r);
                resize(d, d, Size(100, 150));

                int score = INT_MIN;
                int idx = -1;
                for (size_t k = 0; k < digitROI.size(); k++)
                {
                    Mat res;
                    matchTemplate(d, digitROI[k], res, TM_CCOEFF);
                    double minVal, maxVal;
                    minMaxLoc(res, &minVal, &maxVal);
                    if (maxVal > score)
                    {
                        score = maxVal;
                        idx = k;
                    }
                }
                if (score > 20000000)
                    ans.push_back(idx);
            }
            if (ans.size() == 6)
                results.push_back(ans);
            ans.clear();
        }
    }
    //imshow("original", src);
    return results;
}

bool start_cognize(Mat src, std::string _str_file_name)
{
    resize(src, src, Size(360.0, 360.0 * src.rows / src.cols));
    Mat gray;
    cvtColor(src, gray, COLOR_BGR2GRAY);

    auto ref = getRefOCR();
    auto ans = detectDigits(src, gray, ref);

    ofstream myfile;
    myfile.open(_str_file_name);
    
    for (int i = 0; i < ans.size(); i++)
    {
        for (int j = 0; j < ans[i].size(); j++)
        {
            myfile << ans[i][j];
        }
        myfile << ",";
    }

    myfile.close();
    //waitKey(0);

    return true;
}