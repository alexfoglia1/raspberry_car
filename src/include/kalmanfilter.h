#ifndef KALMANFILTER_H
#define KALMANFILTER_H

class KalmanFilter
{
public:
    KalmanFilter()
    {
        Q[0] = 0.001;
        Q[1] = 0.0001;
        R = 0.11;
        meas = 0.0;
        bias = 0.0;
    }

    double getMeasurement(double newMeasure, double newRate, double dt);

    double Q[2];
    double R;
    double meas;
    double bias;
    double rate;
    double P[2][2];
    double K[2];
    double y;
    double S;
};

#endif // KALMANFILTER_H
