#include "lice.h"
#include <math.h>

#define LICE_COMBINE_IMPLEMENT_HSV
#include "lice_combine.h"


LICE_pixel LICE_AlterColorHSV_int(LICE_pixel color, int dH, int dS, int dV)  // H is rolled over [0,384), S and V are clamped [0,255)
{
  int h, s, v;
  LICE_RGB2HSV(LICE_GETR(color), LICE_GETG(color), LICE_GETB(color), &h, &s, &v);
  
  h += dH;
  s += dS;
  v += dV;

  if (h < 0) h += 384;
  else if (h >= 384) h -= 384;
  
  if (s & ~255) 
  {
    if (s<0) s = 0;
    else s = 255;
  }
  
  if (v&~255)
  {
    if (v < 0) v = 0.;
    else v = 255;
  }

  return LICE_HSV2Pix(h, s, v, LICE_GETA(color));
}

LICE_pixel LICE_AlterColorHSV(LICE_pixel color, float dH, float dS, float dV)  // H is rolled over, S and V are clamped, all [0,1)
{
  int dHi = (int)(dH*384.0f);
  int dSi = (int)(dS*255.0f);
  int dVi = (int)(dV*255.0f);
  return LICE_AlterColorHSV_int(color, dHi, dSi, dVi);
}

void LICE_AlterBitmapHSV(LICE_IBitmap* src, float dH, float dS, float dV) // H is rolled over, S and V are clamped
{
  if (src) LICE_AlterRectHSV(src,0,0,src->getWidth(),src->getHeight(),dH,dS,dV);
}

void LICE_AlterRectHSV(LICE_IBitmap* src, int x, int y, int w, int h, float dH, float dS, float dV) // H is rolled over, S and V are clamped
{
  if (!src) return;

  if (x < 0) {
    w += x;
    x = 0;
  }
  if (y < 0) {
    h += y;
    y = 0;
  }
  if (x+w > src->getWidth()) {
    w = src->getWidth()-x;
  }
  if (y+h > src->getHeight()) {
    h = src->getHeight()-y;
  }

  int span = src->getRowSpan();
  LICE_pixel* px = src->getBits()+y*span+x;  

  int dHi = (int)(dH*384.0f);
  int dSi = (int)(dS*255.0f);
  int dVi = (int)(dV*255.0f);
  if (dHi > 383) dHi=383;
  else if (dHi < -383) dHi=-383;


  if (!dHi && !dSi && !dVi) return; // no mod

  if (w*h > 8192)
  {
    // generate a table of HSV translations with clip/clamp
    unsigned char stab[256], vtab[256];
    short htab[384];
    int x;
    for(x=0;x<256;x++)
    {
      int a=x+dSi;
      if(a<0)a=0; else if (a>255)a=255;
      stab[x]=a;

      a=x+dVi;
      if(a<0)a=0; else if (a>255)a=255;
      vtab[x]=a;

      a=x+dHi;
      if(a<0)a+=384; else if (a>=384)a-=384;
      htab[x]=a;
    }
    for(;x<384;x++)
    {
      int a=x+dHi;
      if(a<0)a+=384; else if (a>=384)a-=384;
      htab[x]=a;
    }

    while (h-->0)
    {
      LICE_pixel* tpx = px;
      px+=span;
      int xi=w;
      while (xi-->0)
      {
        LICE_pixel color = *tpx;
        int h,s,v;
        LICE_RGB2HSV(LICE_GETR(color), LICE_GETG(color), LICE_GETB(color), &h, &s, &v);
        *tpx++ = LICE_HSV2Pix(htab[h],stab[s],vtab[v],LICE_GETA(color));
      }
    }
  }
  else
  {
    while (h-->0)
    {
      LICE_pixel* tpx = px;
      px+=span;
      int xi=w;
      while (xi-->0)
        *tpx++ = LICE_AlterColorHSV_int(*tpx, dHi, dSi, dVi);
    }
  }
}