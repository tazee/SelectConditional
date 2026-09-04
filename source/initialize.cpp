#include "edgeByLength.hpp"
#include "edgeByNormal.hpp"
#include "edgeByEdgeVector.hpp"
#include "polyByArea.hpp"
#include "random.hpp"

void initialize()
{
    EdgeByLength::initialize();
    EdgeByNormal::initialize();
    EdgeByEdgeVector::initialize();
    PolyByArea::initialize();
    Random::initialize();
}