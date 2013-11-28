#include "../include/Utility.h"


Utility::Utility()
{
    //ctor
}

Utility::~Utility()
{
    //dtor
}

bool Utility::compareConfidence(const Object& o1, const Object& o2)
{
    return o1.confidence > o2.confidence;
}

bool Utility::compareBoxes(const Rect& r1, const Rect& r2)
{
    return Utility::calcOverlap(r1,r2) > 0.6;
}

bool Utility::compareOverlap(const pair<int,float> o1, const pair<int,float> o2)
{
    return o1.second > o2.second;
}

/**
 * Ermittelt die Box, die alle positiven Boxen umschließt
 */
bool Utility::calcHull(int* grid, vector<pair<int,float> >& posBoxes, Rect& hull)
{
    int x1 = INT_MAX, x2 = 0;
    int y1 = INT_MAX, y2 = 0;
    int id;
    for (size_t i = 0; i < posBoxes.size(); ++i){
        id = posBoxes[i].first;
        x1 = min(grid[id * 4],x1);
        y1 = min(grid[id * 4 + 1],y1);
        x2 = max(grid[id * 4] + grid[id * 4 + 2],x2);
        y2 = max(grid[id * 4 + 1] + grid[id * 4 + 3],y2);
    }
    hull.x = x1;
    hull.y = y1;
    hull.width = x2 - x1;
    hull.height = y2 - y1;
    return true;
}

/**
 * Berechnet die Überlappung zweier Rechtecke
 *
 * TODO: Wenn die cb kommplett in tb liegt, dann gib 1 zurück
 */
float Utility::calcOverlap(const Rect& cb, const Rect& tb)
{
    if (cb.x > tb.x+tb.width) { return 0.0; }
    if (cb.y > tb.y+tb.height) { return 0.0; }
    if (cb.x+cb.width < tb.x) { return 0.0; }
    if (cb.y+cb.height < tb.y) { return 0.0; }

    float colInt =  min(cb.x+cb.width,tb.x+tb.width) - max(cb.x, tb.x);
    float rowInt =  min(cb.y+cb.height,tb.y+tb.height) - max(cb.y,tb.y);

    float intersection = colInt * rowInt;
    float area1 = cb.width*cb.height;
    float area2 = tb.width*tb.height;
    return intersection / (area1 + area2 - intersection);

}

/**
 * Iteriert über das gesamt grid und berechnet die Überlappungen mit bb
 */
void Utility::calcOverlap(int* grid, int& gridSize, Rect& bb, float* overlap)
{
    int bbx = bb.x, bby = bb.y, bbw = bb.width, bbh = bb.height;
    int gbx, gby, gbw, gbh;

    for (int i = 0; i < gridSize; ++i) {
        gbx = grid[i * 4];
        gby = grid[i * 4 + 1];
        gbw = grid[i * 4 + 2];
        gbh = grid[i * 4 + 3];
        if (bbx > gbx+gbw) { overlap[i] = 0.0; continue; }
        if (bby > gby+gbh) { overlap[i] = 0.0; continue; }
        if (bbx+bbw < gbx) { overlap[i] = 0.0; continue; }
        if (bby+bbh < gby) { overlap[i] = 0.0; continue; }
        if(bbx > gbx && bby > gby && bbx + bbw < gbx + gbw && bby + bbh < gby + gbh)
        {
            overlap[i] =  1;
        }
        //cout << "GB:" << gbx << "," << gby << " " << gbw << " " << gbh << endl;
        //cout << "BB:" << bbx << "," << bby << " " << bbw << " " << bbh << endl;

        float colInt =  min(bbx+bbw,gbx+gbw) - max(bbx, gbx);
        float rowInt =  min(bby+bbh,gby+gbh) - max(bby,gby);

        float intersection = colInt * rowInt;
        float area1 = bbw*bbh;
        float area2 = gbw*gbh;
        //cout << intersection << " " << area1 << " " << area2 << endl;
        overlap[i] = intersection / (float)(area1 + area2 - intersection);
    }

}

double Utility::calcVariance(Rect& box,const Mat& sum,const Mat& sqsum){
    //cout << box.x << "," << box.y << " w:" << box.width << " h:" << box.height;
    double brs = sum.at<int>(box.y+box.height,box.x+box.width);
    double bls = sum.at<int>(box.y+box.height,box.x);
    double trs = sum.at<int>(box.y,box.x+box.width);
    double tls = sum.at<int>(box.y,box.x);
    double brsq = sqsum.at<double>(box.y+box.height,box.x+box.width);
    double blsq = sqsum.at<double>(box.y+box.height,box.x);
    double trsq = sqsum.at<double>(box.y,box.x+box.width);
    double tlsq = sqsum.at<double>(box.y,box.x);
    double mean = (brs+tls-trs-bls)/((double)box.area());
    double sqmean = (brsq+tlsq-trsq-blsq)/((double)box.area());

    //cout << " : " << sqmean << ", " << (mean*mean) << endl;
    return sqmean - (mean*mean);
}

float Utility::calcVariance(Mat& patch)
{
    Scalar stdev, mean;
    meanStdDev(patch,mean,stdev);
    return pow(stdev.val[0],2);

}


/** Gecheckt! **/
float Utility::calcMedian(vector<float>& vec)
{
    if (vec.size() == 0)
    {
        cout << "Kein Vector" << endl;
        return 0.0;
    }

    /*sort(vec.begin(),vec.end());
    size_t s = vec.size();
    if (vec.size() % 2 == 0) return (vec[s / 2 - 1] + vec[s / 2]) / 2;
    return vec[s / 2.0];*/
    int n = floor(vec.size() / 2.0);
    nth_element(vec.begin(), vec.begin() + n, vec.end());
    return vec[n];
}

/**
 * Achtung! Hier eine Farbigen Patch übergeben?!
 */
void Utility::generateNormalizedPatch(const Mat &img, Mat &pattern)
{
    Scalar mean;
    Scalar stdev;
//    float m;
    //Ändere Größe
    resize(img,pattern,Size(20,20));
    //Berechne Mittelwert und Standartabweichung
    meanStdDev(pattern,mean,stdev);
    //normalize(pattern,pattern);
    //Konvertiere
    //pattern.convertTo(pattern,CV_32F);
    //Ziehe halben mittlewert ab
    //m = mean.val[0] / 15 * 15;
    pattern = pattern - (mean.val[0] / 4);

}

/**
 * Generiert ein Frame mit einer Menge Images.
 */
bool Utility::generateMultipleImages(vector<Mat>& images, Mat& result, int maxCols = 210, int maxRows = 210, int width = 50, int height = 50)
{
    // Idee: iteriere über alle images.
    // setze neue Größe für result, je nachdem wie groß images[i] ist
    int x = 0, y = 0;
    result = Mat(maxCols,maxRows,images[0].type());
    Mat patch;
    Mat roi;

    for(size_t i = 0; i < images.size(); ++i) {
        /// Wenn das Bild nicht mehr in die Zeile passt, dann beginne in neuer Zeile
        if (x + width > maxCols) {
            x = 0;
            y += height;
        }
        /// Es passt nix mehr ins Bild
        if (y + height> maxRows) {
            return false;
        }
        /// Nun das image[i] skalieren und in Result kopieren
        /// Die Hauptidee ist ein ROI in Result zu definieren (genau an die Stelle x,y result.rows - resizeRow, result.cols - resizeCol)
        /// und dann zu kopieren
        // Set the image ROI to display the current image
        // Resize the input image and copy the it to the Single Big Image
        resize(images[i],patch,Size(width,height));
        roi = result(Rect(x, y, width,height));
        //cout << x << "," << y << " " << resizeRow << "," << resizeCol << " ResultSIze:" << result.rows << "," << result.cols << endl;
        patch.copyTo(roi);
        x += width;

    }

    return true;
}

