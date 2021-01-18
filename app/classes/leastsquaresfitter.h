#ifndef LEASTSQUARESFITTER_H
#define LEASTSQUARESFITTER_H

#include <dlib/optimization.h>
#include <QtCore>

#define LEAST_SQUARES_N_FITS 20   // number of iterations
#define LEAST_SQUARES_LIMIT_FACTOR 1.5
#define LEAST_SQUARES_MAX_ITERATIONS 75

typedef dlib::matrix<double,1,1> input_vector;
typedef dlib::matrix<double,6,1> parameter_vector;

/*!
 * \brief The LeastSquaresFitter class
 * base class for fitting nonlinear functions to a given data sample with dlib
 */
class LeastSquaresFitter
{
public:
    enum class Type {SUPERPOS};

    LeastSquaresFitter();

    double model(double input) const;

    void solve(const std::vector<std::pair<double, double>>& samples, int nIterations = LEAST_SQUARES_N_FITS, double limitFactor = LEAST_SQUARES_LIMIT_FACTOR);
    void solve_lm(const std::vector<std::pair<double, double>>& samples, int nIterations = LEAST_SQUARES_N_FITS, double limitFactor = LEAST_SQUARES_LIMIT_FACTOR);

    double residual_sum_of_sqares(const std::vector<std::pair<double, double> > &samples) const;

    virtual double tau_90() = 0;
    virtual double f_t_90() = 0;

    std::vector<double> getParams() const;

    QList<QString> getParameterNames() const;

    QStringList getTooltips() const;

    virtual QString getFunctionString() const = 0;

    virtual Type type() const = 0;

    static QMap<QString, LeastSquaresFitter::Type> getTypeMap();

    bool parameters_valid(double y_limit) const;

    void resetParams();

protected:
    parameter_vector params;
    QList<QString> parameterNames;
    static QMap<QString, Type> typeMap;

    double residual_sum_of_sqares(const std::vector<std::pair<double, double> > &samples, const parameter_vector &parameters) const;

    virtual double model(
        const input_vector& input_vector,
        const parameter_vector& parameter_vector
    ) const = 0;

    virtual parameter_vector residual_derivative(
        const std::pair<input_vector, double>& data,
        const parameter_vector& parameter_vector
    ) = 0;

    double residual(
        const std::pair<input_vector, double>& data,
        const parameter_vector& params
    ) const;

    virtual parameter_vector getRandomParameterVector(const std::vector<std::pair<double, double>>& samples) const = 0;

    virtual bool parameters_valid(const parameter_vector &param_vector, double y_limit) const = 0;
};

/*!
 * \brief The ADG_superpos_Fitter class
 * implements superposition of two "Asymptotic regression models"
 * f: alpha1 * (1 - e^(-beta1 * (t - t01)) + alpha2 * (1 - e^(-beta2 * (t - t02))
 */
class ADG_superpos_Fitter : public LeastSquaresFitter
{
public:
    ADG_superpos_Fitter();

    double tau_90() override;
    double f_t_90() override;

    Type type() const { return Type::SUPERPOS; }

    virtual QString getFunctionString() const override;

protected:
    double model(
        const input_vector& input_vector,
        const parameter_vector& parameter_vector
    ) const;


    parameter_vector residual_derivative(
        const std::pair<input_vector, double>& data,
        const parameter_vector& parameter_vector
    );

    parameter_vector getRandomParameterVector(const std::vector<std::pair<double, double>>& samples) const;

    virtual bool parameters_valid(const parameter_vector &param_vector, double y_limit) const override;
};

class LinearFitter
{
public:
    LinearFitter();

    double model(const double input) const;
    void fit (const std::vector<double> &x, const std::vector<double> &y);

    double getM() const;

    double getB() const;

    double getStdDev() const;

private:
    double m = 0., b = 0., stdDev = 0.;
};

#endif // LEASTSQUARESFITTER_H
