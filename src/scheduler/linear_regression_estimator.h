//
// Created by robert on 13/2/18.
//

#ifndef NVM_LINEAR_REGRESSION_ESTIMATOR_H
#define NVM_LINEAR_REGRESSION_ESTIMATOR_H
#include "ready_state_estimator.h"

class linear_regression_estimator: public ready_state_estimator {
    virtual int estimate_number_of_ready_reads(uint64_t timestamp) {
        uint64_t now = ticks();
        int counts[granularity * 2];
        for(int i = 0 ; i < granularity * 2; i++) {
            counts[i] = 0;
        }

        for(auto it = read_time_array_.cbegin(); it != read_time_array_.cend(); ++it) {
            int offset = max(0, (int)(width - cycles_to_microseconds(now - *it))) / (width / granularity);
            counts[offset]++;
        }

        for(auto it = write_time_array_.cbegin(); it != write_time_array_.cend(); ++it) {
            int offset = max(0, (int)(width - cycles_to_microseconds(now - *it))) / (width / granularity);
            counts[offset + granularity]++;
        }

        float sum = 0;
        for (int i = 0; i < granularity * 2; i++) {
            sum += read_para[i] * counts[i];
        }

        return (int) sum;
    }

    virtual int estimate_number_of_ready_writes(uint64_t timestamp) {
        uint64_t now = ticks();
        int counts[granularity * 2];
        for(int i = 0 ; i < granularity * 2; i++) {
            counts[i] = 0;
        }

        for(auto it = read_time_array_.cbegin(); it != read_time_array_.cend(); ++it) {
            int offset = max(0, (int)(width - cycles_to_microseconds(now - *it))) / (width / granularity);
            counts[offset]++;
        }

        for(auto it = write_time_array_.cbegin(); it != write_time_array_.cend(); ++it) {
            int offset = max(0, (int)(width - cycles_to_microseconds(now - *it))) / (width / granularity);
            counts[offset + granularity]++;
        }

        float sum = 0;
        for (int i = 0; i < granularity * 2; i++) {
            sum += write_para[i] * counts[i];
        }

        return (int) sum;
    }

private:

    const float read_para[granularity * 2] = {8.25852811e-01,  1.01035404e+00,  1.88915089e-01,
                                              3.97996068e-01,  7.50082910e-01,  1.03489280e+00,
                                              3.71338576e-01,  1.05456996e+00,  8.54490161e-01,
                                              9.81397927e-01,  6.11372828e-01,  7.16356754e-01,
                                              9.30878878e-01,  9.18818533e-01,  9.76585388e-01,
                                              9.19179142e-01,  8.97882223e-01,  5.23032725e-01,
                                              4.09765616e-02, -3.55157368e-02,  5.73257841e-02,
                                              -5.12599945e-06,  6.55651093e-07,  0.00000000e+00,
                                              0.00000000e+00,  0.00000000e+00,  0.00000000e+00,
                                              0.00000000e+00,  0.00000000e+00,  0.00000000e+00,
                                              0.00000000e+00,  0.00000000e+00,  0.00000000e+00,
                                              0.00000000e+00,  0.00000000e+00,  0.00000000e+00,
                                              -6.97796488e+00, -9.07931998e-02,  4.90838438e-02};
    const float write_para[granularity * 2] = {-6.41523860e-04,  8.84994864e-04,  8.67053121e-03,
                                               5.60705457e-03,  3.32736154e-03, -4.55116620e-03,
                                               4.23709722e-03,  2.66929367e-03,  8.52044765e-03,
                                               -8.10625905e-04,  5.72307967e-03, -5.40810451e-03,
                                               -5.61975921e-03,  3.51363211e-04,  1.15470856e-03,
                                               8.73622543e-04,  3.21616186e-04, -5.66797680e-04,
                                               2.19752709e-03,  1.10178231e-03,  3.14030536e-02,
                                               7.15255737e-07, -8.88248906e-08,  0.00000000e+00,
                                               0.00000000e+00,  0.00000000e+00,  0.00000000e+00,
                                               0.00000000e+00,  0.00000000e+00,  0.00000000e+00,
                                               0.00000000e+00,  0.00000000e+00,  0.00000000e+00,
                                               0.00000000e+00,  0.00000000e+00,  0.00000000e+00,
                                               9.77653086e-01,  1.00882912e+00,  9.82268393e-01};

};

#endif //NVM_LINEAR_REGRESSION_ESTIMATOR_H
