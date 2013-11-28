#include "../include/VarianceClassifier.h"

VarianceClassifier::VarianceClassifier()
{
    variance = 0.0;
    //ctor
}

VarianceClassifier::~VarianceClassifier()
{
    //dtor
}


/**
 * Die Methode berechnet die integralen Bilder für die Klassifizierung.
 * Jedes Bild muss hier reingeworfen werden, damit der Klassfizierer dieses als Grundlage nutzen kann.
 */
void VarianceClassifier::calcIntegral(const Mat& f)
{
    integral(f,integralImg,integralImgSqrt);
    //Scalar stdev, mean;
    //meanStdDev(f1(bb),mean,stdev);
    //variance = pow(stdev.val[0],2) * 0.5;
}

bool VarianceClassifier::classify(int& index, int* data, Result* resultSet)
{
    int x = data[index * 4], y = data[index * 4 + 1], width = data[index * 4 + 2], height = data[index * 4 + 3];
    //cout << "x=" << x << " y=" << y << " width=" << width << " height=" << height << endl;
    double brs = integralImg.at<int>(y + height, x + width);
    double bls = integralImg.at<int>(y + height, x);
    double trs = integralImg.at<int>(y, x + width);
    double tls = integralImg.at<int>(y, x);
    double brsq = integralImgSqrt.at<double>(y + height, x + width);
    double blsq = integralImgSqrt.at<double>(y + height, x);
    double trsq = integralImgSqrt.at<double>(y, x + width);
    double tlsq = integralImgSqrt.at<double>(y, x);
    double mean = (brs+tls-trs-bls)/((double)(width * height));
    double sqmean = (brsq+tlsq-trsq-blsq)/((double)(width * height));
    resultSet->variances[index] = sqmean-mean*mean;
    //cout << variance << " > " << resultSet->variances[index] << endl;
    return resultSet->variances[index] >= variance;

}

bool VarianceClassifier::classify(Object& o, const Mat& f)
{
    Mat patch = f(o.getRect()).clone();
    float var = Utility::calcVariance(patch);
    o.variance = var;
    //var = var;
    //if (var >= variance) cout << "MINVAR: " << variance << " x: " << bb.x << " y:" << bb.y << " w:" << bb.width << " h:" << bb.height << " variance: " << var << endl;
    return var >= (variance);
}

/**
 * Jedes Objekt hat eine List mit den IDs von den Objekten, die gleichzeitig
 * gesucht werden.
 */
bool VarianceClassifier::multi_classify(Object& o, Mat& f)
{
    bool stillOk = false;
    int id = -1;
    // Varianz des Patches berechnen
    Mat patch = f(o.getRect());
    float var = Utility::calcVariance(patch);
    //Iteriere über alle IDs der Objekte. Entsprechend werden die varianzWerte geprüft
    for (size_t i = 0; i < o.getObjectIds().size(); ++i) {
        id = o.getObjectIds().at(i);
        if (var >= variances[id]) {
            stillOk = true;
        } else {
            // Falls Objekt mit Id id raus fällt, dann lösche id aus Objektlist
            o.getObjectIds().erase(o.getObjectIds().begin() + id);
        }
    }

    return stillOk;

}
