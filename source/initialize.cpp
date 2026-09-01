#include "edgeByLength.hpp"
#include "edgeByNormal.hpp"
#include "edgeByAngle.hpp"
#include "polyByArea.hpp"
#include "random.hpp"

void initialize()
{
    EdgeByLength::initialize();
    EdgeByNormal::initialize();
    EdgeByAngle::initialize();
    PolyByArea::initialize();
    Random::initialize();
}