#include "Insets.h"

using namespace Sexy;

Insets::Insets() : mLeft(0), mTop(0), mRight(0), mBottom(0) {}

Insets::Insets(int theLeft, int theTop, int theRight, int theBottom)
    : mLeft(theLeft), mTop(theTop), mRight(theRight), mBottom(theBottom) {}
