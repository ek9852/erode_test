#include <RenderScript.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
 
#include "erode_rs.h"
#include "ScriptC_erode3x3.h"

using android::RSC::Allocation;
using android::RSC::Element;
using android::RSC::RS;
using android::RSC::Type;
using android::RSC::sp;

static sp<RS> rs;
static sp<ScriptC_erode3x3> sc;
static sp<Allocation> ain;
static sp<Allocation> aout;

int
erode3x3_rs_init(int w, int h)
{
    rs = new RS();

    // only legitimate because this is a standalone executable
    // cache directory, must be writable
    rs->init("/data/local/tmp");

    sp<const Element> e = Element::U8(rs);

    Type::Builder tb(rs, e);
    tb.setX(w);
    tb.setY(h);
    sp<const Type> t = tb.create();

    ain = Allocation::createTyped(rs, t);
    aout = Allocation::createTyped(rs, t);

    sc = new ScriptC_erode3x3(rs);
    sc->set_gIn(ain);
    sc->set_gWidth(w);
    sc->set_gHeight(h);

    return 0;
}

void
erode3x3_rs(uint8_t *in_data, uint8_t *out_data, int w, int h)
{
    ain->copy2DStridedFrom(in_data, w * sizeof(uint8_t));

    sc->forEach_root(aout);

    aout->copy2DStridedTo(out_data, w * sizeof(uint8_t));
}

void
erode3x3_rs_destroy()
{
    sc.clear();
    ain.clear();
    aout.clear();
    rs.clear();
}
