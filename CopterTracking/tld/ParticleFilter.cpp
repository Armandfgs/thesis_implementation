#include "../include/ParticleFilter.h"


ParticleFilter::ParticleFilter()
{
    /// Die Parameter initialisieren
    vector_size = 100;
    dynamical_vector = vector_size;
    messurement_vector = vector_size;
    number_of_samples = 100;
    //ctor
}

ParticleFilter::~ParticleFilter()
{
    //dtor
}

void ParticleFilter::init()
{
    cout << "Initialisierung des ParticleFilters" << endl;
    // Das condensation-Objekt
    // Diese Struktur hält alle Daten aus der Messung und ist wie folgt aufgebaut
    /**
    CvConDensation
    {
        int MP; // Dimension of measurement vector
        int DP; // Dimension of state vector
        float* DynamMatr; // Matrix of the linear Dynamics system
        float* State; // Vector of State
        int SamplesNum; // Number of Samples
        float** flSamples; // array of the Sample Vectors
        float** flNewSamples; // temporary array of the Sample Vectors
        float* flConfidence; // Confidence for each Sample
        float* flCumulative; // Cumulative confidence
        float* Temp; // Temporary vector
        float* RandomSample; // RandomVector to update sample set
        CvRandState* RandS; // Array of structures to generate random vectors
    } CvConDensation;

    */
    condensation = cvCreateConDensation(dynamical_vector,messurement_vector,number_of_samples);
    // Zuallererst muss sie initialisiert werden. Das geht mittels einer Funktion, kann aber auch per Hand gelöst werden
    CvMat* lowerBound = cvCreateMat((int)vector_size,1,CV_32F); //lowerBound = lowerBound;
    CvMat* upperBound = cvCreateMat((int)vector_size,1,CV_32F); //upperBound = upperBound;
    // die Boundaries müssen nun noch initialisiert werden (das mache ich mal so wie im OpenCV-Buch)
    for (size_t i = 0; i < vector_size; ++i) {
        lowerBound->data.fl[i] = -1.0f;
        upperBound->data.fl[i] = 1.0f;
    }
    cvConDensInitSampleSet(condensation,lowerBound,upperBound);
    cout << "DONE!";
}

bool ParticleFilter::track(Rect& bb, Mat& frame)
{
    // An dieser Stelle müssen die posteriors mittels Wahrscheinlichkeitsverteilung gesetzt werden (WIRKLICH??).
    // Ich nehme die Gaußsche Verteilung aus dem Buch. Vielleicht ists ja ok.
    float* messurements = new float[condensation->MP];
    updateConfidence(messurements);
    //Und hier werden die Partikel "re-sampled"
    cvConDensUpdateByTime( condensation );

    return true;
}

void ParticleFilter::updateConfidence(float* messure)
{
    for( int i=0; i < condensation->SamplesNum; i++ ) {
        float p = 1.0f;
        for( int j=0; j < condensation->DP; j++ ) {
            p *= (float) exp(
            -0.05*(messure[j] - condensation->flSamples[i][j])*(messure[j]-condensation->flSamples[i][j])
            );
        }
        condensation->flConfidence[i] = .5;
    }
}
