#include <LatAnalyze/Functional/CompiledModel.hpp>
#include <LatAnalyze/Numerical/GslMinimizer.hpp>
#include <LatAnalyze/Core/Plot.hpp>
#include <LatAnalyze/Statistics/XYSampleData.hpp>

using namespace std;
using namespace Latan;

const Index  nPoint1 = 10, nPoint2 = 10;
const Index  nSample = 1000;
const double xErr = .1, yErr   = .3;
const double exactPar[2] = {0.5,5.};
const double dx1 = 10.0/static_cast<double>(nPoint1);
const double dx2 = 5.0/static_cast<double>(nPoint2);

int main(void)
{
    // generate fake data
    XYSampleData          data(nSample);
    double                xBuf[2];
    random_device         rd;
    mt19937               gen(rd());
    normal_distribution<> dis;
    DoubleModel           f([](const double *x, const double *p)
                            {return p[1]*exp(-x[0]*p[0])+x[1];}, 2, 2);
    
    cout << "-- generating fake data..." << endl;
    data.addXDim(nPoint1);
    data.addXDim(nPoint2);
    data.addYDim();
    for (Index s = central; s < nSample; ++s)
    {
        for (Index i1 = 0; i1 < nPoint1; ++i1)
        {
            xBuf[0]          = i1*dx1;
            data.x(i1, 0)[s] = xErr*dis(gen) + xBuf[0];
            for (Index i2 = 0; i2 < nPoint2; ++i2)
            {
                xBuf[1]                              = i2*dx2;
                data.x(i2, 1)[s]                     = xBuf[1];
                data.y(data.dataIndex(i1, i2), 0)[s] = yErr*dis(gen)
                                                       + f(xBuf, exactPar);
            }
        }
    }
    data.assumeXExact(true, 1);
    
    // set minimizers
    DVec            init = DVec::Constant(2, 0.1);
    SampleFitResult p;
    GslMinimizer    min(GslMinimizer::Algorithm::bfgs2);
    
    // fit
    cout << "-- fit..." << endl;
    f.parName().setName(0, "m");
    f.parName().setName(1, "A");
    p = data.fit(min, init, f);
    p.print();
    
    // plot
    Plot       plot;
    DVec       ref(2);
    XYStatData res;
    
    cout << "-- generating plots..." << endl;
    ref(1) = 0.;
    res    = data.getPartialResiduals(p, ref, 0).getData();
    plot << PlotRange(Axis::x, 0., 10.);
    plot << Color("rgb 'blue'");
    plot << PlotPredBand(p.getModel(_).bind(0, ref), 0., 10.);
    plot << Color("rgb 'blue'");
    plot << PlotFunction(p.getModel().bind(0, ref), 0., 10.);
    plot << Color("rgb 'red'");
    plot << PlotData(res);
    plot.display();
    
    return EXIT_SUCCESS;
}
