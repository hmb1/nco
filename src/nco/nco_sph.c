/*
This code is described in "Computational Geometry in C" (Second Edition),
Chapter 7.  It is not written to be comprehensible without the
explanation in that book.

Written by Joseph O'Rourke.
Last modified: December 1997
Questions to orourke@cs.smith.edu.
--------------------------------------------------------------------
This code is Copyright 1997 by Joseph O'Rourke.  It may be freely
redistributed in its entirety provided that this copyright notice is
not removed.
--------------------------------------------------------------------
*/

#include "nco_sph.h"

/* global variables for latitude, longitude in RADIANS
   these may be set in nco_poly.c or
   should be safe with OPenMP   */

int DEBUG_SPH=0;

static double LAT_MIN_RAD;
static double LAT_MAX_RAD;

static double LON_MIN_RAD;
static double LON_MAX_RAD;






void nco_sph_prn(double **sR, int r, int istyle)
{
  int idx;


  printf("\nSpherical Polygon\n");

  for( idx = 0; idx < r; idx++ )
  for( idx = 0; idx < r; idx++ )
     nco_sph_prn_pnt(">", sR[idx], istyle, True);
    //printf("%20.14f %20.14f\n", sR[idx][0], sR[idx][1]);

  printf("End Polygon\n");


}


/* spherical functions */
int nco_sph_intersect(poly_sct *P, poly_sct *Q, poly_sct *R, int *r)
{
  const char fnc_nm[]="nco_sph_intersect()";

   nco_bool qpFace = False;
   nco_bool pqFace = False;
   nco_bool isGeared = False;

   int numIntersect=0;

   int n;
   int m;

   int a = 0, a1 = 0, aa=0;
   int b = 0, b1 = 0, bb=0;


   int ipqLHS = 0;
   int ip1qLHS = 0 ;
   int iqpLHS = 0;
   int iq1pLHS = 0 ;

   nco_bool isParallel=False;

   double nx1;
   double nx2;
   double nx3;
   double dx1;


   char code='0';

   double Pcross[NBR_SPH];
   double Qcross[NBR_SPH];
   double Xcross[NBR_SPH];

   double p[NBR_SPH];
   double q[NBR_SPH];

   poly_vrl_flg_enm inflag= poly_vrl_unk;

   n=P->crn_nbr;
   m=Q->crn_nbr;

   if(DEBUG_SPH)
      fprintf(stdout, "%s: just entered %s\n", nco_prg_nm_get(), fnc_nm);


   do{


      a1 = (a + n - 1) % n;
      b1 = (b + m - 1) % m;



      nx1= nco_sph_cross(P->shp[a1], P->shp[a], Pcross);
      nx2= nco_sph_cross(Q->shp[b1], Q->shp[b], Qcross);

      nx3= nco_sph_cross(Pcross, Qcross, Xcross);


      ipqLHS = nco_sph_lhs(P->shp[a], Qcross);
      ip1qLHS = nco_sph_lhs(P->shp[a1], Qcross);


      /* imply rules facing if 0 */

      if(ipqLHS==0 && ip1qLHS!=0)
         ipqLHS=ip1qLHS*-1;
      else if( ipqLHS != 0 && ip1qLHS == 0 )
         ip1qLHS=ipqLHS*-1;


      iqpLHS = nco_sph_lhs(Q->shp[b], Pcross);
      iq1pLHS = nco_sph_lhs(Q->shp[b1], Pcross);

      /* imply rules facing if 0 */


      if(iqpLHS == 0 && iq1pLHS != 0)
         iqpLHS=iq1pLHS*-1;
      else if(iqpLHS != 0 && iq1pLHS == 0)
         iq1pLHS=iqpLHS*-1;


      /* now calculate face rules */
      qpFace = nco_sph_face(ip1qLHS, ipqLHS, iqpLHS);
      pqFace = nco_sph_face(iq1pLHS, iqpLHS, ipqLHS);

      /* Xcross product near zero !! so make it zero*/
      dx1=1.0- nco_sph_dot_nm(Pcross,Qcross );

      /* spans parallel but in oposite directions */
      if( fabs(dx1-2.0) < DOT_TOLERANCE )
        return EXIT_FAILURE;

      if( dx1  <DOT_TOLERANCE )
      {

         ip1qLHS=0;
         ipqLHS=0;
         iq1pLHS=0;
         iqpLHS=0;
         qpFace=0;
         pqFace=0;

         isParallel=True;
      }
      else
        isParallel=False;


      if( isGeared == False)
      {
         if(  (ipqLHS == 1 && iqpLHS == 1) ||  ( qpFace && pqFace )     )
         {
            aa++;a++;
         }
         else
         {
            isGeared = True;
         }
      }





      if(isGeared)
      {

        if(isParallel)
        {
          poly_vrl_flg_enm lcl_inflag = poly_vrl_unk;

          code = nco_sph_seg_parallel(P->shp[a1], P->shp[a], Q->shp[b1], Q->shp[b], p, q, &lcl_inflag);

          if (lcl_inflag != poly_vrl_unk ) {

            inflag = lcl_inflag;

            /* there is a subtle  trick here - a point is "force added" by setting the flags pqFace and qpFace */
            if (code == '2')
              nco_sph_add_pnt(R->shp, r, p);

            if (inflag == poly_vrl_pin) {
              pqFace = 1;
              qpFace = 0;

            } else if (inflag == poly_vrl_qin) {
              pqFace = 0;
              qpFace = 1;
            }

            if (numIntersect++ == 0) {
              /* reset counters */
              aa = 0;
              bb = 0;
            }
          }


        }

        if(!isParallel) {
          code = nco_sph_seg_int(P->shp[a1], P->shp[a], Q->shp[b1], Q->shp[b], p, q);


          if (code == '1' || code == 'e') {

            nco_sph_add_pnt(R->shp, r, p);

            if (numIntersect++ == 0) {
              /* reset counters */
              aa = 0;
              bb = 0;
            }

            inflag = (ipqLHS == 1 ? poly_vrl_pin : iqpLHS == 1 ? poly_vrl_qin : inflag);


            if (DEBUG_SPH)
              printf("%%InOut sets inflag=%s\n", nco_poly_vrl_flg_sng_get(inflag));

          }
        }

         if(DEBUG_SPH)
            printf("numIntersect=%d code=%c (ipqLHS=%d, ip1qLHS=%d), (iqpLHS=%d, iq1pLHS=%d), (qpFace=%d pqFace=%d)\n",numIntersect, code, ipqLHS, ip1qLHS,  iqpLHS,iq1pLHS, qpFace,pqFace);



         if (qpFace && pqFace)  {

            /* Advance either P or Q which has previously arrived ? */
            if(inflag == poly_vrl_pin) nco_sph_add_pnt(R->shp,r, P->shp[a]);

            aa++;a++;


         } else if (qpFace) {
            if(inflag == poly_vrl_qin) nco_sph_add_pnt(R->shp,r, Q->shp[b]);

            bb++;b++;


            /* advance q */
         } else if (pqFace) {
            /* advance p */
            if(inflag == poly_vrl_pin) nco_sph_add_pnt(R->shp,r,P->shp[a]);

            aa++;a++;

         } else if (iqpLHS == -1) {
            /* advance q */
            //if(inflag== Qin) sAddPoint(R,r,Q->shp[b]);
            bb++;b++;

            /* cross product zero  */
         } else if( ipqLHS==0 && ip1qLHS==0 && iq1pLHS ==0 && iqpLHS ==0   ){
            if(inflag==poly_vrl_pin)
            {bb++;b++;}
            else
            {aa++;a++;}

         }



         else {
            /* catch all */
            if(inflag==poly_vrl_pin) nco_sph_add_pnt(R->shp,r,P->shp[a]);
            aa++;a++;

         }

      }

      a%=n;
      b%=m;

      if(DEBUG_SPH)
         fprintf(stdout, "\ndebug isGeared=%d a=%d aa=%d b=%d bb=%d \n",isGeared, a, aa, b, bb);

      /* quick exit if current point is same a First point  - nb an exact match ?*/
      //if( *r >3 &&  R->shp[0][3]==R->shp[*r-1][3] && R->shp[0][4]==R->shp[*r-1][4] )
      if( *r >3 &&  1.0 - nco_sph_dot_nm(R->shp[0], R->shp[*r-1]) < DOT_TOLERANCE  )
      {
         --*r;
         break;
      }


   } while ( ((aa < n) || (bb < m)) && (aa < 2*n) && (bb < 2*m) );

   return EXIT_SUCCESS;

}


char  nco_sph_seg_int(double *a, double *b, double *c, double *d, double *p, double *q)
{
  const char fnc_nm[]="nco_sph_seg_int()";

  int flg_sx=0;

  double nx1;
  double nx2;
  double nx3;
  double nx_ai;
  double nx_ci;

  double dx_ab;
  double dx_ai;
  double dx_ib;

  double dx_cd;
  double dx_ci;
  double dx_id;

  double darc;

  double  Pcross[NBR_SPH]={0};
  double  Qcross[NBR_SPH]={0};
  double  Icross[NBR_SPH]={0};
  double   ai[NBR_SPH]={0};
  double   ci[NBR_SPH]={0};



  if(flg_sx) {
    nx1= nco_sph_sxcross(a, b, Pcross);
    nx2= nco_sph_sxcross(c, d, Qcross);

    nco_sph_add_lonlat(Pcross);
    nco_sph_add_lonlat(Qcross);

    nx3= nco_sph_cross(Pcross, Qcross, Icross);
    nco_sph_add_lonlat(Icross);
  }
  else
  {
    nx1= nco_sph_cross(a, b, Pcross);
    nx2= nco_sph_cross(c, d, Qcross);

    nx3= nco_sph_cross(Pcross, Qcross, Icross);
    nco_sph_add_lonlat(Icross);
  }

  darc=atan(nx3);

  /*
  if(DEBUG_SPH) {
    nco_sph_prn_pnt("nco_sph_seg_int(): intersection", Icross, 3, True);
    printf("%s: ||Pcross||=%.20g ||Qcross||=%.20g ||Icross||=%.20g arc=%.20g\n",fnc_nm,  nx1, nx2, nx3, darc);
  }
  */

  /* Icross is zero, should really have a range rather than an explicit zero */
  /* use dot product to se if Pcross and QCross parallel */
  if(  1.0- nco_sph_dot_nm(Pcross,Qcross )  <DOT_TOLERANCE  )
    //return nco_sph_parallel(a, b, c, d, p, q);
    return '0';



  dx_ab=1.0 - nco_sph_dot_nm(a,b);


  dx_cd=1.0 - nco_sph_dot_nm(c,d);

  dx_ai=1.0-  nco_sph_dot_nm(a,Icross);

  if(dx_ai < DOT_TOLERANCE )
     dx_ai=0.0;
  else
     nx_ai=nco_sph_cross(a, Icross, ai);

  dx_ci= 1.0- nco_sph_dot_nm(c,Icross);

  if(dx_ci <DOT_TOLERANCE )
    dx_ci=0.0;
  else
    nx_ci=nco_sph_cross(c, Icross, ci);



  if(0 && DEBUG_SPH)
    fprintf(stderr,"%s(): dx_ab=%2.10f dx_ai=%2.10f  nx1=%2.20f nx_ai=%2.10f   \n", fnc_nm, dx_ab, dx_ai, nx1, nx_ai );

  if(  ( dx_ai==0.0 ||  (  nco_sph_dot_nm(ai, Pcross) >0.99 && dx_ai>= 0.0 && dx_ai<=dx_ab  )) &&
       ( dx_ci==0.0 ||  (  nco_sph_dot_nm(ci, Qcross) >0.99 && dx_ci>0.0 && dx_ci <= dx_cd  ) )
    )
  {
    nco_sph_add_lonlat(Icross);

    if(DEBUG_SPH)
      nco_sph_prn_pnt("nco_sph_seg_int(): intersection", Icross, 3, True);

    memcpy(p,Icross, sizeof(double)*NBR_SPH);
    return '1';

  }


  /* try antipodal point */
  Icross[0]*= -1.0;
  Icross[1]*= -1.0;
  Icross[2]*= -1.0;


  dx_ai=1.0-  nco_sph_dot_nm(a,Icross);

  if(dx_ai !=0.0 )
    nx_ai=nco_sph_cross(a, Icross, ai);

  dx_ci= 1.0- nco_sph_dot_nm(c,Icross);

  if(dx_ci !=0.0 )
    nx_ci=nco_sph_cross(c, Icross, ci);



  if(0 && DEBUG_SPH)
    fprintf(stderr,"%s(): dx_ab=%2.10f dx_ai=%2.10f  nx1=%2.20f nx_ai=%2.10f   \n", fnc_nm, dx_ab, dx_ai, nx1, nx_ai );

  if(  ( dx_ai==0.0 ||  (  nco_sph_dot_nm(ai, Pcross) >0.99 && dx_ai>= 0.0 && dx_ai<=dx_ab  )) &&
       ( dx_ci==0.0 ||  (  nco_sph_dot_nm(ci, Qcross) >0.99 && dx_ci>0.0 && dx_ci <= dx_cd  ) )
  )
  {
    nco_sph_add_lonlat(Icross);
    if(DEBUG_SPH)
      nco_sph_prn_pnt("nco_sph_seg_int(): intersect-antipodal", Icross, 3, True);

    memcpy(p,Icross, sizeof(double)*NBR_SPH);
    return '1';

  }





  return '0';


}


char
nco_sph_seg_parallel(double *p0, double *p1, double *q0, double *q1, double *r0, double *r1, poly_vrl_flg_enm *inflag )
{

  const char fnc_nm[] = "nco_sph_seg_parallel()";

  char code;
  int flg_sx = 0;

  double nx1;
  double nx2;
  double nx3;

  double dx_p1;
  double dx_q0;
  double dx_q1;


  double Pcross[NBR_SPH] = {0};
  double Qcross[NBR_SPH] = {0};
  double Tcross[NBR_SPH] = {0};


  if (flg_sx) {
    nx1 = nco_sph_sxcross(p0, p1, Pcross);
    nx2 = nco_sph_sxcross(q0, q1, Qcross);

    nco_sph_add_lonlat(Pcross);
    nco_sph_add_lonlat(Qcross);


  } else {
    nx1 = nco_sph_cross(p0, p1, Pcross);
    nx2 = nco_sph_cross(q0, q1, Qcross);

  }

  /* check points in the same direction */
  if (nco_sph_dot_nm(Pcross, Qcross) < 0.99)
    return '0';

  dx_p1 = 1.0 - nco_sph_dot_nm(p0, p1);

  dx_q0 = 1.0 - nco_sph_dot_nm(p0, q0);

  if( dx_q0< DOT_TOLERANCE)
    dx_q0=0.0;


  if (dx_q0 != 0.0) {
    nx3 = nco_sph_cross(p0, q0, Tcross);

    if (nco_sph_dot_nm(Pcross, Tcross) < 0.0)
      dx_q0 *= -1.0;

  }

  dx_q1 = 1.0 - nco_sph_dot_nm(p0, q1);

  if(dx_q1 <DOT_TOLERANCE)
    dx_q1=0.0;

  if (dx_q1 != 0.0) {
    nx3 = nco_sph_cross(p0, q1, Tcross);

    if (nco_sph_dot_nm(Pcross, Tcross) < 0.0)
      dx_q1 *= -1.0;
  }

  /* we now have 4 "points to order"
  * a=0.0, dx_ab, dx_ac , dx_ad
   * always dx_ab > 0.0 and dx_ac < dx_ad
   * */

  /* no overlap so return */
  if( (dx_q0 < 0.0  && dx_q1 < 0.0) || ( dx_q0 > dx_p1 && dx_q1 > dx_p1  )) {
    code = '0';
    return code;
  }

  if(dx_q0 <0.0 &&  dx_q1 == 0.0   )
  {
    code='1';
    nco_sph_adi(r0,p0);
    /* not sure which flag to set here */
    *inflag=poly_vrl_qin;
  }
  else if( dx_q0 == dx_p1 && dx_q1 > dx_p1  )
  {
    code='1';
    nco_sph_adi(r0,p1);
    *inflag=poly_vrl_pin;
  }
    /* LHS overlap */
  else if (dx_q0 <0.0 &&  (dx_q1 >0.0 && dx_q1 <= dx_p1)  ) {
    code= '2';
    nco_sph_adi(r0, p0);
    nco_sph_adi(r1, q1);
    *inflag=poly_vrl_qin;

  }
    /* RHS overlap */
  else if( dx_q0 >=0.0 &&  dx_q0 < dx_p1 && dx_q1 > dx_p1    )
  {
    code= '2';
    nco_sph_adi(r0, q0);
    nco_sph_adi(r1, p1);
    *inflag=poly_vrl_pin;

  }
  else if(  dx_q0 >=0.0 && dx_q1 <= dx_p1    ) {
    code= '2';
    nco_sph_adi(r0, q0);
    nco_sph_adi(r1, q1);
    *inflag=poly_vrl_qin;
  }
  else if( dx_q0 <0.0 && dx_q1 > dx_p1    )
  {
    code='2';
    nco_sph_adi(r0,p0 );
    nco_sph_adi(r0,p1 );
    *inflag=poly_vrl_pin;
  } else{
    code='0';
  }

  if(DEBUG_SPH )
  {
    if (code >= '1')
      nco_sph_prn_pnt("nco_sph_seg_parallel(): intersect1", r0, 3, True);

    if (code == '2')
      nco_sph_prn_pnt("nco_sph_seg_parallel(): intersect2", r1, 3, True);

  }


  return code;
}



/* returns true if vertex is on edge (a,b) */
nco_bool
nco_sph_seg_vrt_int(double *a, double *b, double *vtx)
{
  double nx_ab;
  double nx_av;

  double dx_ab;
  double dx_av;

  double  Pcross[NBR_SPH]={0};
  double  Vcross[NBR_SPH]={0};


  nx_ab=nco_sph_sxcross(a, b, Pcross);

  dx_ab=1.0 - nco_sph_dot_nm(a,b);

  dx_av=1.0 - nco_sph_dot_nm(a,vtx);

  if(dx_av >0.0  )
    nx_av=nco_sph_cross(a, vtx, Vcross );


  if( nco_sph_dot_nm(Pcross, Vcross) >0.9999 && dx_av >=0.0 && dx_av <= dx_ab )
    return True;

  return False;


}






char  nco_sph_seg_int_1(double *a, double *b, double *c, double *d, double *p, double *q)
{
  const char fnc_nm[]="nco_shp_seg_int()";

  int flg_sx=0;

   double nx1;
   double nx2;
   double nx3;

   double darc;

   double  Pcross[NBR_SPH]={0};
   double  Qcross[NBR_SPH]={0};
   double  Icross[NBR_SPH]={0};



   if(flg_sx) {
      nx1= nco_sph_sxcross(a, b, Pcross);
      nx2= nco_sph_sxcross(c, d, Qcross);

     nco_sph_add_lonlat(Pcross);
     nco_sph_add_lonlat(Qcross);

      nx3= nco_sph_cross(Pcross, Qcross, Icross);
     nco_sph_add_lonlat(Icross);
   }
   else
   {
      nx1= nco_sph_cross(a, b, Pcross);
      nx2= nco_sph_cross(c, d, Qcross);

      nx3= nco_sph_cross(Pcross, Qcross, Icross);
     nco_sph_add_lonlat(Icross);
   }

   darc=atan(nx3);

   if(DEBUG_SPH) {
      nco_sph_prn_pnt("nco_sph_seg_int(): intersection", Icross, 3, True);
      printf("%s: ||Pcross||=%.20g ||Qcross||=%.20g ||Icross||=%.20g arc=%.20g\n",fnc_nm,  nx1, nx2, nx3, darc);
   }

   /* Icross is zero, should really have a range rather than an explicit zero */
   if( nx3 < 1.0e-15)
      return nco_sph_parallel(a, b, c, d, p, q);


   if(nco_sph_lonlat_between(a, b, Icross) && nco_sph_lonlat_between(c, d, Icross) )
   {
      memcpy(p,Icross, sizeof(double)*NBR_SPH);
      return '1';
   }

   /* try antipodal point */
   Icross[0]*= -1.0;
   Icross[1]*= -1.0;
   Icross[2]*= -1.0;

   nco_sph_add_lonlat(Icross);

   if(nco_sph_lonlat_between(a, b, Icross) && nco_sph_lonlat_between(c, d, Icross) )
   {

      memcpy(p,Icross, sizeof(double)*NBR_SPH);
      return '1';
   }

   return '0';





}






/* takes a point and a cross product representing the normal to the arc plane */
/* returns 1 if point on LHS of arc plane */
/* returns -1 if point on RHS of arc plane */
/* return 0 if point on the arc - (given suitable tolerances ) */
int nco_sph_lhs(double *Pi, double *Qi)
{
   double ds;

   ds= nco_sph_dot(Pi, Qi);



   if(ds  > 0.0 )
      return 1;
   else if(ds <0.0)
      return -1;
   else
      return 0;


   /*
   ds=acos( nco_sph_dot(Pi,Qi) );

   if( ds < M_PI_2 - ARC_MIN_LENGTH )
     return 1;
   else if ( ds > M_PI_2 + ARC_MIN_LENGTH )
     return -1;
   else
     return 0;
  */

}

/* implement face rules */
nco_bool nco_sph_face(int iLHS, int iRHS, int jRHS)
{
   if( iLHS == 1 && iRHS == -1 && jRHS == -1 )
      return True;

   if( iLHS == -1 && iRHS == 1 && jRHS == 1  )
      return True;

   return False;


}



double  nco_sph_dot(double *a, double *b)
{
   int idx;
   double sum=0.0;

   for(idx=0; idx<3; idx++)
      sum+=a[idx]*b[idx];

   return sum;


}

/* dot product normalized */
double  nco_sph_dot_nm(double *a, double *b)
{
  int idx;
  double sum=0.0;
  double n1;
  double n2;

  const char fnc_nm[]="nco_sph_dot_nm()";

  for(idx=0; idx<3; idx++)
    sum+=a[idx]*b[idx];

  n1=sqrt( a[0]*a[0]+a[1]*a[1] + a[2]*a[2] );
  n2=sqrt( b[0]*b[0]+b[1]*b[1] + b[2]*b[2] );

   sum= (sum / n1) / n2;

  if(0 && DEBUG_SPH)
    fprintf(stderr,"%s() dt=%f n1=%f %f\n", fnc_nm, sum, n1, n2 );


  return sum;


}




double  nco_sph_cross(double *a, double *b, double *c)
{
  const char fnc_nm[]="nco_sph_cross()";
   //
   double n1;

   c[0]=a[1]*b[2]-a[2]*b[1];
   c[1]=a[2]*b[0]-a[0]*b[2];
   c[2]=a[0]*b[1]-a[1]*b[0];

   // normalize vector
   n1=sqrt( c[0]*c[0]+c[1]*c[1] + c[2]*c[2] );

   if( n1 >  0.0 && n1 != 1.0  )
   {
      c[0] /= n1;
      c[1] /= n1;
      c[2] /= n1;
   }

   if(0 && DEBUG_SPH)
      printf("%s: n1=%f (%f, %f %f)\n",fnc_nm, n1, c[0],c[1], c[2]);

   return n1;

}

double nco_sph_rad(double *a){
  double n1;

  n1=sqrt( a[0]*a[0]+a[1]*a[1] + a[2]*a[2] );

  return n1;
}


/* new method for calculating cross product */
double nco_sph_sxcross(double *a, double *b, double *c)
{
  nco_bool bDeg = False;
  double n1;
  double lon1;
  double lon2;

  double lat1;
  double lat2;

  if (bDeg) {
    lon1 = a[3] * M_PI / 180.0;
    lat1 = a[4] * M_PI / 180.0;

    lon2 = b[3] * M_PI / 180.0;
    lat2 = b[4] * M_PI / 180.0;
  } else{
    lon1 = a[3];
    lat1 = a[4];

    lon2 = b[3];
    lat2 = b[4];

  }


   c[0] =   sin(lat1+lat2) * cos( (lon1+lon2) / 2.0) * sin( (lon1-lon2)/2.0)
            - sin(lat1-lat2) * sin ((lon1+lon2) / 2.0) * cos( (lon1-lon2)/2.0);

   c[1] =   sin(lat1+lat2) * sin( (lon1+lon2) / 2.0) * sin( (lon1-lon2)/2.0)
            + sin(lat1-lat2) * cos ((lon1+lon2) / 2.0) * cos( (lon1-lon2)/2.0);



   c[2]=cos(lat1) * cos(lat2) * sin(lon2-lon1);


   // normalize vector
   n1=sqrt( c[0]*c[0]+c[1]*c[1] + c[2]*c[2] );

   if( n1 != 0.0 && n1 !=1.0  )
   {
      c[0] /= n1;
      c[1] /= n1;
      c[2] /= n1;
   }

   if(DEBUG_SPH)
      printf("sxCross(): n1=%f (%f, %f %f)\n", n1, c[0],c[1], c[2]);

   return n1;

}


void  nco_sph_adi(double *a, double *b)
{
   (void)memcpy(a,b, sizeof(double)* NBR_SPH);
}






void nco_sph_add_pnt(double **R, int *r, double *P)
{

   double delta;

   //delta = ( *r==0 ? 0.0 :   2.0 *asin(    sqrt( pow( R[*r-1][0] - P[0],2 ) + pow( R[*r-1][1] - P[1],2 ) + pow( R[*r-1][2] - P[2],2 )  ) /2.0) );
   if(*r >0 )
      delta = 1.0 - nco_sph_dot(R[*r-1], P );

   if(DEBUG_SPH)
      nco_sph_prn_pnt("aAddPoint():", P, 3, True);



   /* only add  point if its distinct from previous point */
   if ( *r==0 ||  delta > DOT_TOLERANCE )
   {

      memcpy(R[*r], P, sizeof(double)*NBR_SPH);
      (*r)++;
   }


}


nco_bool nco_sph_between(double a, double b, double x)
{
  const char fnc_nm[]="nco_sph_between()";


   nco_bool bret=False;

   double diff;

   diff=fabs(b-a);



   if(diff==0.0 )
   {


     if( fabs(x-a)<= SIGMA_RAD )
        bret=True;
   }
   else if( diff <= SIGMA_RAD )
   {

      if(  ( b >a &&  x>= a && x<=b ) || ( b<a && x>=b && x<=a )   )
         bret= True;

   }
   else if( diff < M_PI )
   {

      if(  ( b >a &&  x>= a && x<=b ) || ( b<a && x>=b && x<=a )   )
         bret=True;

   }
   /* this indicates a wrapped cell (or edge )
    * this same code works for domains (0-360) and (-180,180) */
   else if( diff >  M_PI )
   {

     if( (b>a && (x>=b || x<=a)) || ( b<a && (x<=b ||  x>=a)) )
        bret= True;
   }




  if(DEBUG_SPH)
    printf("%s: a=%.20f, b=%.20f, x=%.20f %s \n",fnc_nm, a, b, x, (bret==True ? "True":"False"));

   return bret;


}





/* use crt coords to check bounds */
nco_bool nco_sph_lonlat_between(double *a, double *b, double *x)
{
  const char fnc_nm[]="nco_sph_lonlat_between()";

   /* working in radians here */
   nco_bool bDeg=False;
   nco_bool bRet=False;

   double lat_min;
   double lat_max;

   if(nco_sph_between(a[3], b[3], x[3]) == False )
      return False;

   /* special lat check */
   //getLatCorrect(a,b, &lat_min,&lat_max);
  nco_geo_get_lat_correct(a[3], a[4], b[3], b[4], &lat_min, &lat_max, bDeg);



   if( x[4]>=lat_min && x[4]<=lat_max )
      bRet=True;
   else
      bRet=False;

  if(DEBUG_SPH)
    printf("%s: lat_min=%.20f lat_max=%.20f lat=%.20f %s\n",fnc_nm, lat_min, lat_max, x[4],
           (bRet ? "True" : "False") );




  return bRet;


}

nco_bool sxBetween(double *a, double *b, double *c)
{

   if ( a[3] != b[3] )
      return (  ( c[3] >= a[3] && c[3] <=b[3] ) || ( c[3] <= a[3] && c[3] >= b[3] )) ;
   else
      return (  ( c[4] >= a[4] && c[4] <=b[4] ) || ( c[4] <= a[4] && c[4] >= b[4] )) ;


   /*
   if ( a[3] != b[3] )
     return (  a[3] <= c[3] && b[3] >= c[3] ||  a[3] >= c[3] && b[3] <= c[3] ) ;
   else
     return (  a[4] <= c[4] && b[4] >=c[4]   ||  a[4] >= c[4] && b[4] <= c[4] ) ;
   */

}


int
nco_sph_parallel_lat(double *p1, double *p2, double *q1, double *q2, double *a, double *b)
{
  bool bdir=False;

  /* check sense of direction */
  bdir=( p2[4] - p1[4] >0.0);

  if( (q2[4] - q1[4] >0.0) != bdir  )
     return 0;


  // if( nco_sph_between(p1[4], p2[4], q1[4]) && nco_sph_between(p1[4], p2[4], q2[4])  )



}




int nco_sph_parallel(double *a, double *b, double *c, double *d, double *p, double *q)
{

   char code='0';
   const char *ptype="none";

   if( sxBetween( a, b, c ) && sxBetween( a, b, d ) ) {
      nco_sph_adi(p, c);
      nco_sph_adi(q, d);
      ptype="abc-abd";
      code= 'e';
   }
   else if( sxBetween( c, d, a ) && sxBetween( c, d, b ) ) {
      nco_sph_adi(p, a);
      nco_sph_adi(q, b);
      ptype="cda-cdb";
      code= 'e';
   }
   else if( sxBetween( a, b, c ) && sxBetween( c, d, b ) ) {
      nco_sph_adi(p, c);
      nco_sph_adi(q, b);
      ptype="abc-cdb";
      code= 'e';
   }
   else if( sxBetween( a, b, c ) && sxBetween( c, d, a ) ) {
      nco_sph_adi(p, c);
      nco_sph_adi(q, a);
      ptype="abc-cda";
      code= 'e';
   }
   else if( sxBetween( a, b, d ) && sxBetween( c, d, b ) ) {
      nco_sph_adi(p, d);
      nco_sph_adi(q, b);
      ptype="abd-cdb";
      code= 'e';
   }
   else if( sxBetween( a, b, d ) && sxBetween( c, d, a ) ) {
      nco_sph_adi(p, d);
      nco_sph_adi(q, a);
      ptype="abd-cda";
      code= 'e';
   }

   if(DEBUG_SPH)
      printf("sParallelDouble(): code=%c type=%s\n", code, ptype);

   return code;
}



void nco_sph_prn_pnt(const char *sMsg, double *p, int style, nco_bool bRet)
{

   printf("%s ", sMsg);

   switch(style)
   {
      case 0:
      default:
         printf( "(dx=%.20f, dy=%.20f, dz=%.20f), (lon=%.20f,lat=%.20f)",p[0], p[1], p[2], p[3], p[4] );
       break;

      case 1:
         printf( "(dx=%.20f, dy=%.20f, dz=%.20f)",p[0], p[1], p[2] );
       break;

      case 2:
         printf( "(lon=%.20f,lat=%.20f)",p[3], p[4] );
       break;

      case 3:
         printf( "(lon=%.20f,lat=%.20f)",p[3] *180.0/M_PI,  p[4]*180/M_PI );
       break;

      case 4:
         printf( "(dx=%.20f, dy=%.20f, dz=%.20f), (lon=%.20f,lat=%.20f)",p[0], p[1], p[2], p[3] *180.0/M_PI,  p[4]*180/M_PI);
       break;

      case 5:
         printf( "(dx=%f, dy=%f, dz=%f), (lon=%f,lat=%f)",p[0], p[1], p[2], p[3] *180.0/M_PI,  p[4]*180/M_PI);
       break;



   }

   if(bRet)
      printf("\n");
   else
      printf(" * ");

}

nco_bool nco_sph_is_convex(double **sP, int np)
{
  const char fnc_nm[]="nco_sph_is_convex()";


nco_bool flg_sx=0;

int idx;
int idx_pre;
int idx_nex;


double n1;
double n2;

double dp;
double theta;
double rad1_nco=1.0;
double rad=1.0;

double  aCross[NBR_SPH];
double  bCross[NBR_SPH];

for(idx=0; idx<np;idx++)
{
  idx_pre=(idx + np -1)% np;
  idx_nex=(idx + np +1)% np;

  if(flg_sx) {
    n1 = nco_sph_sxcross(sP[idx], sP[idx_pre], aCross);
    n2 = nco_sph_sxcross(sP[idx], sP[idx_nex], bCross);
  } else {
    n1 = nco_sph_cross(sP[idx], sP[idx_pre], aCross);
    n2 = nco_sph_cross(sP[idx], sP[idx_nex], bCross);


  }

  //rad1_nco = sRadius(aCross);
  //rad  = sRadius(bCross);
  dp= nco_sph_dot(aCross, bCross);


  // dp=sDot(sP[idx1], sP[idx]) / rad1_nco /rad;
  theta=acos(dp);

  if(DEBUG_SPH)
    printf("%s():, %d angle=%f, dp=%f, n1=%.15g n2=%.15g\n", fnc_nm, idx, theta*180.0/M_PI, dp, n1, n2);


  //if( fabs(theta - M_PI) >SIGMA_RAD )
  //   return False;


}

return True;


}

/* make a control point that is Outside of polygon */
int nco_sph_mk_control(poly_sct *sP, double* pControl  )
{
   /* do stuff in radians */


   int iret=NCO_ERR;
   double clat=0.0;
   double clon=0.0;

   nco_bool bDeg=False;

   /* convert limits to radians */
   double lon_min=D2R( sP->dp_x_minmax[0]);
   double lon_max=D2R( sP->dp_x_minmax[1]);
   double lat_min=D2R( sP->dp_y_minmax[0]);
   double lat_max=D2R( sP->dp_y_minmax[1]);

   double xbnd=D2R(8.0);


   /* polar cap */
   if( sP->bwrp && sP->bwrp_y )
   {
     /* get latitude of equator */
     double lat_eq= (LAT_MAX_RAD-LAT_MIN_RAD) /2.0;

     /* choose an arbitary lon  */
     clon=D2R(20);

     /* check if we have an north or south pole *
      * nb a north polar cap - all points in nothern hemisphere
      *    a south polar cap all point in southern hemisphere */

     if(lat_min >=lat_eq && lat_max > lat_eq  )
       clat=lon_min-xbnd / 2.0;
     else if( lat_min < lat_eq && lat_max <= lat_eq  )
       clat=lon_max+xbnd / 2.0;
     else
       return NCO_ERR;

   }

   /* just longitude wrapping */
   else if(sP->bwrp)
   {
      /* nb distance between lmin and lmax  >180.0 */
      clon=lon_min+xbnd / 2.0;
      clat=( lat_min+lat_max ) /2.0;
   }
   /* no wrapping x */
   else {
       /* choose left or right hand size */
       if (lon_min - LON_MIN_RAD > xbnd) {
         clon = lon_min - xbnd / 2.0;
         clat = (lat_min + lat_max) / 2.0;

       } else if (LON_MAX_RAD - lon_max > xbnd) {

         clon = lon_max + xbnd / 2.0;
         clat = (lat_min + lat_max) / 2.0;

       }
         /* choose below or above */
       else if (lat_min - LAT_MIN_RAD > xbnd) {
         clat = lat_min - xbnd / 2.0;
         /* choose centre */
         clon = (lon_min + lon_max) / 2.0;

       } else if (LAT_MAX_RAD - lat_max > xbnd) {
         clat = lat_max + xbnd / 2.0;
         clon = (lon_min + lon_max) / 2.0;


       } else {
         return NCO_ERR;
       }
   }

   /* remember clat, clon in radians */
   nco_geo_lonlat_2_sph(clon, clat, pControl, bDeg );

   return NCO_NOERR;


}


/* nb doesnt work if polygon spans more than 180.0
 * works by counting the number of intersections of the
   line (pControl, pVertex) and each edge in sP
   pControl is chosen so that it is OUTSIDE of sP
 */
nco_bool nco_sph_pnt_in_poly(double **sP, int n, double *pControl, double *pVertex)
{

  char code;
  int idx;
  int idx1=0;
  int numIntersect=0;

  double  p[NBR_SPH];
  double  q[NBR_SPH];


  /* count number of intersections */
  for(idx=0; idx< n ; idx++)
  {
    idx1=(idx+n -1) % n ;

    code= nco_sph_seg_int(sP[idx1], sP[idx], pControl, pVertex, p, q);

    if(code=='1' || code=='v' || code == 'e')
      numIntersect++;


  }

  /* for any polygon (convex or concave)
    an odd  number of crossings means that the point is inside
    while an even number means that it is outside */

  return (numIntersect % 2  );


}




/* set static globals */
void nco_sph_set_domain(double lon_min_rad, double lon_max_rad, double lat_min_rad, double lat_max_rad)
{

  LON_MIN_RAD=lon_min_rad;
  LON_MAX_RAD=lon_max_rad;

  LAT_MIN_RAD=lat_min_rad;
  LAT_MAX_RAD=lat_max_rad;

  return;

}


void
nco_sph_add_lonlat(double *ds)
{
 nco_bool bDeg=False;

 nco_geo_sph_2_lonlat(ds, &ds[3], &ds[4], bDeg);

}



/*------------------------ nco_geo functions manipulate lat & lon  ----------------------------------*/

/* assume latitude -90,90 */
double nco_geo_lat_correct(double lat1, double lon1, double lon2)
{

   double dp;

   if( fabs(lon1 - lon2) <= SIGMA_RAD || fabs(lat1) <= SIGMA_RAD || lat1 >= LAT_MAX_RAD - SIGMA_RAD   || lat1 <= LAT_MIN_RAD + SIGMA_RAD  )
      return lat1;

   //lat1=lat1*M_PI / 180.0;

   /* exact constant is  is 2.0 but lets be a bit generous */
   dp= tan(lat1) / cos ( fabs(lon2-lon1) / 2.0 ) ;

   dp=atan(dp);


   return dp;


}





void nco_geo_get_lat_correct(double lon1, double lat1, double lon2, double lat2, double *dp_min, double *dp_max,
                             nco_bool bDeg)
{

  double dswp;

   if( lat2 >lat1 )
   {
      dswp=lat1;
      lat1=lat2;
      lat2=dswp;
   }

   if(lon1>lon2)
   {

     dswp=lon1;
     lon1=lon2;
     lon2=dswp;

   }


   if(bDeg)
   {
      lat1 *= M_PI / 180.0;
      lat2 *= M_PI / 180.0;
      lon1 *= M_PI / 180.0;
      lon2 *= M_PI / 180.0;
   }

  /* deal with wrpping . nb this code works for (-180, 180 ) ( 0 - 360) */
  if(lon2-lon1 >= M_PI)
    lon2-=2*M_PI;



  /* lat1 & lat2 >0.0 */
   if( lat1>0.0 && lat2 >=0.0)
   {
      *dp_max = nco_geo_lat_correct(lat1, lon1, lon2);
      *dp_min = lat2;
   }
   else if( lat1 <= 0.0 && lat2<0.0 )
   {
      *dp_max = lat1;
      *dp_min = nco_geo_lat_correct(lat2, lon1, lon2);
   }

   else if( lat1 >0.0 && lat2  < 0.0)
   {
      *dp_max= nco_geo_lat_correct(lat1, lon1, lon2);
      *dp_min= nco_geo_lat_correct(lat2, lon1, lon2);

   }
   else
   {
      *dp_max=0.0;
      *dp_min=0.0;

   }

   /* convert back to degrees */
   if(bDeg)
   {
      *dp_max *= 180.0 / M_PI;
      *dp_min *= 180.0 / M_PI;
   }

   return;



}


/* assumes lon, lat in degrees */
void nco_geo_lonlat_2_sph(double lon, double lat, double *b, nco_bool bDeg)
{

   if(bDeg) {
      lon *= M_PI / 180.0;
      lat *= M_PI / 180.0;
   }

   b[0] = cos(lat) * cos(lon);
   b[1] = cos(lat) * sin(lon);
   b[2] = sin(lat);

   /* lat lon - we need this for bounding box */
   b[3] = lon;
   b[4] = lat;

}


void  nco_geo_sph_2_lonlat(double *a, double *lon, double *lat, nco_bool bDeg)
{

   /* nb this returns range (-180, 180) */
   *lon = atan2(a[1],a[0]) ;

   if( *lon < 0.0 &&  LON_MIN_RAD >=0.0  )
      *lon+= (LON_MAX_RAD);

   // b[1]= asin(a[2]) * 180.0 /M_PI;
   *lat=atan2( a[2], sqrt( a[0]*a[0]+a[1]*a[1] ) ) ;

   /* convert to degrees if required */
   if(bDeg)
   {
      *lon*=(180.0 / M_PI );
      *lat*=(180.0 / M_PI );

   }

   return;
}



/****************  functions for RLL grids *******************************************/

int nco_rll_intersect(poly_sct *P, poly_sct *Q, poly_sct *R, int *r)
{
  const char fnc_nm[]="nco_rll_intersect()";

  nco_bool qpFace = False;
  nco_bool pqFace = False;
  nco_bool isGeared = False;

  int numIntersect=0;

  int n;
  int m;

  int a = 0, a1 = 0, aa=0;
  int b = 0, b1 = 0, bb=0;


  int ipqLHS = 0;
  int ip1qLHS = 0 ;
  int iqpLHS = 0;
  int iq1pLHS = 0 ;

  nco_bool isParallel=False;
  nco_bool isP_LatCircle=False;
  nco_bool isQ_LatCircle=False;

  double nx1;
  double nx2;
  double nx3;


  char code='0';

  double Pcross[NBR_SPH];
  double Qcross[NBR_SPH];
  double Xcross[NBR_SPH];

  double p[NBR_SPH];
  double q[NBR_SPH];

  poly_vrl_flg_enm inflag= poly_vrl_unk;

  n=P->crn_nbr;
  m=Q->crn_nbr;

  if(DEBUG_SPH)
    fprintf(stdout, "%s: just entered %s\n", nco_prg_nm_get(), fnc_nm);


  do {


    a1 = (a + n - 1) % n;
    b1 = (b + m - 1) % m;

    isP_LatCircle = nco_rll_is_lat_circle(P->shp[a1], P->shp[a]);
    isQ_LatCircle = nco_rll_is_lat_circle(Q->shp[b1], Q->shp[b]);

    nx1 = nco_sph_cross(P->shp[a1], P->shp[a], Pcross);
    nx2 = nco_sph_cross(Q->shp[b1], Q->shp[b], Qcross);

    //nx3= nco_sph_cross(Pcross, Qcross, Xcross);

    if (isQ_LatCircle) {

      ip1qLHS = nco_rll_lhs(P->shp[a1], Q->shp[b1], Q->shp[b] );
      ipqLHS = nco_rll_lhs(P->shp[a], Q->shp[b1],   Q->shp[b]);
    } else {
      ip1qLHS = nco_sph_lhs(P->shp[a1], Qcross);
      ipqLHS = nco_sph_lhs(P->shp[a], Qcross);

    }

    /* imply rules facing if 0 */

    if (ipqLHS == 0 && ip1qLHS != 0)
      ipqLHS = ip1qLHS * -1;
    else if (ipqLHS != 0 && ip1qLHS == 0)
      ip1qLHS = ipqLHS * -1;


    if (isP_LatCircle) {
      iq1pLHS = nco_rll_lhs(Q->shp[b1], P->shp[a1],  P->shp[a]);
      iqpLHS = nco_rll_lhs(Q->shp[b], P->shp[a1], P->shp[a]);


    } else {

      iq1pLHS = nco_sph_lhs(Q->shp[b1], Pcross);
      iqpLHS = nco_sph_lhs(Q->shp[b], Pcross);

    }

    /* imply rules facing if 0 */
    if (iqpLHS == 0 && iq1pLHS != 0)
      iqpLHS = iq1pLHS * -1;
    else if (iqpLHS != 0 && iq1pLHS == 0)
      iq1pLHS = iqpLHS * -1;


    /* now calculate face rules */
    qpFace = nco_sph_face(ip1qLHS, ipqLHS, iqpLHS);
    pqFace = nco_sph_face(iq1pLHS, iqpLHS, ipqLHS);


    /* check for parallel segments */


    /* see if arcs are parallel */
    if (isP_LatCircle && isQ_LatCircle && P->shp[a1][4] == Q->shp[b1][4])
    {
      isParallel = True;
    }
    else if (!isP_LatCircle && !isQ_LatCircle && P->shp[a1][3] == Q->shp[b1][3])
    {
      /* check arc both in same "direction" */
      if (  (P->shp[a][4] > P->shp[a1][4]) != (Q->shp[b][4] > P->shp[b1][4])   )
        return EXIT_FAILURE;
      else
        isParallel = True;
    }
    else
      isParallel = False;

    if (isParallel) {

      ip1qLHS = 0;
      ipqLHS = 0;
      iq1pLHS = 0;
      iqpLHS = 0;
      qpFace = 0;
      pqFace = 0;


    }


    if( isGeared == False)
    {
      if(  (ipqLHS == 1 && iqpLHS == 1) ||  ( qpFace && pqFace )     )
      {
        aa++;a++;
      }
      else
      {
        isGeared = True;
      }
    }





    if(isGeared)
    {

      if(isParallel)
      {
        poly_vrl_flg_enm lcl_inflag = poly_vrl_unk;



        code = nco_rll_seg_parallel(P->shp[a1], P->shp[a], Q->shp[b1], Q->shp[b], p, q, &lcl_inflag);

        if(code=='X')
        {

          if(nco_dbg_lvl_get() >= nco_dbg_dev )
            fprintf(stderr, "%s:%s()\n", nco_prg_nm_get(), fnc_nm, "parallel edges in opposite directions\n");
          return EXIT_FAILURE;
        }



        if (lcl_inflag != poly_vrl_unk ) {

          inflag = lcl_inflag;

          /* there is a subtle  trick here - a point is "force added" by setting the flags pqFace and qpFace */
          if (code == '2')
            nco_sph_add_pnt(R->shp, r, p);

          if (inflag == poly_vrl_pin) {
            pqFace = 1;
            qpFace = 0;

          } else if (inflag == poly_vrl_qin) {
            pqFace = 0;
            qpFace = 1;
          }

          if (numIntersect++ == 0) {
            /* reset counters */
            aa = 0;
            bb = 0;
          }
        }


      }

      if(!isParallel) {
        code = nco_rll_seg_int(P->shp[a1], P->shp[a], Q->shp[b1], Q->shp[b], p, q);


        if (code == '1' || code == 'e') {

          nco_sph_add_pnt(R->shp, r, p);

          if (numIntersect++ == 0) {
            /* reset counters */
            aa = 0;
            bb = 0;
          }

          inflag = (ipqLHS == 1 ? poly_vrl_pin : iqpLHS == 1 ? poly_vrl_qin : inflag);


          if (DEBUG_SPH)
            printf("%%InOut sets inflag=%s\n", nco_poly_vrl_flg_sng_get(inflag));

        }
      }

      if(DEBUG_SPH)
        printf("numIntersect=%d code=%c (ipqLHS=%d, ip1qLHS=%d), (iqpLHS=%d, iq1pLHS=%d), (qpFace=%d pqFace=%d)\n",numIntersect, code, ipqLHS, ip1qLHS,  iqpLHS,iq1pLHS, qpFace,pqFace);



      if (qpFace && pqFace)  {

        /* Advance either P or Q which has previously arrived ? */
        if(inflag == poly_vrl_pin) nco_sph_add_pnt(R->shp,r, P->shp[a]);

        aa++;a++;


      } else if (qpFace) {
        if(inflag == poly_vrl_qin) nco_sph_add_pnt(R->shp,r, Q->shp[b]);

        bb++;b++;


        /* advance q */
      } else if (pqFace) {
        /* advance p */
        if(inflag == poly_vrl_pin) nco_sph_add_pnt(R->shp,r,P->shp[a]);

        aa++;a++;

      } else if (iqpLHS == -1) {
        /* advance q */
        //if(inflag== Qin) sAddPoint(R,r,Q->shp[b]);
        bb++;b++;

        /* cross product zero  */
      } else if( ipqLHS==0 && ip1qLHS==0 && iq1pLHS ==0 && iqpLHS ==0   ){
        if(inflag==poly_vrl_pin)
        {bb++;b++;}
        else
        {aa++;a++;}

      }



      else {
        /* catch all */
        if(inflag==poly_vrl_pin) nco_sph_add_pnt(R->shp,r,P->shp[a]);
        aa++;a++;

      }

    }

    a%=n;
    b%=m;

    if(DEBUG_SPH)
      fprintf(stdout, "\ndebug isGeared=%d a=%d aa=%d b=%d bb=%d \n",isGeared, a, aa, b, bb);

    /* quick exit if current point is same a First point  - nb an exact match ?*/
    //if( *r >3 &&  R->shp[0][3]==R->shp[*r-1][3] && R->shp[0][4]==R->shp[*r-1][4] )
    if( *r >3 &&  1.0 - nco_sph_dot_nm(R->shp[0], R->shp[*r-1]) < DOT_TOLERANCE  )
    {
      --*r;
      break;
    }


  } while ( ((aa < n) || (bb < m)) && (aa < 2*n) && (bb < 2*m) );

  return EXIT_SUCCESS;

}

nco_bool
nco_rll_is_lat_circle(double *p0, double *p1) {

  if (p0[3] != p1[3] && p0[4] == p1[4])
    return True;

  return False;

}

int
nco_rll_lhs(double *p0, double *q0, double *q1)
{

  int iret;
  double nx=1;



  if(q0[3] > q1[3] )
    nx=-1;

  if(fabs(q0[3] - q1[3]) > M_PI)
    nx*=-1.0;

  if(p0[4] > q0[4])
    iret=1;
  else if(p0[4] < q0[4] )
    iret=-1;
  else
    iret=0;

  /* reverse direction maybe*/
  return iret*nx;

}


char
nco_rll_seg_int(double *p0, double *p1, double *q0, double *q1, double *r0, double *r1)
{

  char code='0';
  nco_bool bDeg=False;
  nco_bool isP_LatCircle=False;
  nco_bool isQ_LatCircle=False;

  isP_LatCircle=nco_rll_is_lat_circle(p0, p1);
  isQ_LatCircle=nco_rll_is_lat_circle(q0, q1);


  /* longitude P may hit small ciricle Q */
  if(!isP_LatCircle && isQ_LatCircle )
  {
    /* Check longitude range */
    if( nco_sph_between(q0[3], q1[3], p0[3] )  &&  nco_sph_between(p0[4], p1[4], q0[4] ) ) {
      r0[3] = p0[3];
      r0[4] = q0[4];
      code ='1';
    }

  }
    /* longitude Q may hit small circle P */
  else if(isP_LatCircle && !isQ_LatCircle)
  {
    /* Check range range */
    if( nco_sph_between(p0[3], p1[3], q0[3] )  &&  nco_sph_between(q0[4], q1[4], p0[4] ) ) {
      r0[3] = q0[3];
      r0[4] = p0[4];
      code='1';
    }

  }

  if(code =='1')
    nco_geo_lonlat_2_sph(r0[3], r0[4], r0, bDeg);


  return code;


}

char
nco_rll_seg_parallel(double *p0, double *p1, double *q0, double *q1, double *r0, double *r1, poly_vrl_flg_enm *inflag ) {

  int id;

  char code = '0';

  int p_sgn=1;
  int q_sgn=1;

  nco_bool isP_LatCircle = False;
  nco_bool isQ_LatCircle = False;


  isP_LatCircle = nco_rll_is_lat_circle(p0, p1);
  isQ_LatCircle = nco_rll_is_lat_circle(q0, q1);

  /* check for brain dead call */
  if (isP_LatCircle != isQ_LatCircle)
    return code;


  if (isP_LatCircle)
    id = 3;
  else
    id = 4;

  /* check sense of direction - they must be the same for both spans */
  /* check a longitude span
  if(!isP_LatCircle){

     if( ! (p0[id] > p1[id] != q0[id] > q1[id]) )
       return 'X';

  }
  else{

     if(p0[id] < p1[id])
        p_sgn*=-1;

     if(fabs(p1[id] - p0[id]) > M_PI)
        p_sgn*=-1.0;

     if(q0[id] < p1[id] )
        q_sgn*=-1.0;

    if(fabs(q1[id] - q0[id]) > M_PI)
      p_sgn*=-1.0;


    if(p_sgn != q_sgn )
      return 'X';

  }
  */


  if ( !nco_sph_between(p0[id], p1[id], q0[id]) && nco_sph_between(p0[id], p1[id], q1[id]) ) {
    nco_sph_adi(r0, p0);
    nco_sph_adi(r1, q1);
    *inflag=poly_vrl_qin;
    code = '2';
  } else if ( nco_sph_between(p0[id], p1[id], q0[id]) && !nco_sph_between(p0[id], p1[id], q1[id]) ) {
    nco_sph_adi(r0, q0);
    nco_sph_adi(r1, p1);
    *inflag=poly_vrl_pin;
    code = '2;';

  } else if ( nco_sph_between(p0[id], p1[id], q0[id]) && nco_sph_between(p0[id], p1[id], q1[id])) {
    nco_sph_adi(r0, q0);
    nco_sph_adi(r0, q1);
    *inflag=poly_vrl_qin;
    code = '2';
  } else if (nco_sph_between(q0[id], q1[id], p0[id]) && nco_sph_between(q0[id], q1[id], p1[id])) {
    nco_sph_adi(r0, p0);
    nco_sph_adi(r1, p1);
    *inflag=poly_vrl_pin;
    code = '2';

  } else
    code = '0';

  return code;

}


void nco_rll_add_pnt(double **R, int *r, double *P)
{


  if(DEBUG_SPH)
    nco_sph_prn_pnt("nco_rll_add_pnt()", P, 3, True);


  memcpy(R[*r], P, sizeof(double)*NBR_SPH);
  (*r)++;




}

/*****************************************************************************************/
