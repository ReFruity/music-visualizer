#include <valarray>
#include <complex>

#define PI 3.14159265358979323846264338328L

typedef std::complex<double> Complex;
typedef std::valarray<Complex> CArray;

void fft(CArray &x);
