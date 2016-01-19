#include "../MassParticle.h"


int main(int argc, char *argv[])
{
    int ctx = mpCreateContext();
    mpDestroyContext(ctx);
    return 0;
}
