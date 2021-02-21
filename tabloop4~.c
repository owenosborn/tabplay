
#include "m_pd.h"

/******************** tabplay4~ ***********************/

static t_class *tabplay4_tilde_class;

typedef struct _tabplay4_tilde
{
    t_object x_obj;
    int x_npoints;
    t_word *x_vec;
    t_symbol *x_arrayname;
    t_float x_speed;
    t_float x_loop_start;
    t_float x_loop_end;
    double x_index;
} t_tabplay4_tilde;

static void *tabplay4_tilde_new(t_symbol *s)
{
    t_tabplay4_tilde *x = (t_tabplay4_tilde *)pd_new(tabplay4_tilde_class);
    x->x_arrayname = s;
    x->x_vec = 0;
    outlet_new(&x->x_obj, gensym("signal"));
    x->x_speed = 1;
    x->x_index = 1;
    x->x_loop_start = 0;
    x->x_loop_end = 10000;
    return (x);
}

static t_int *tabplay4_tilde_perform(t_int *w)
{
    t_tabplay4_tilde *x = (t_tabplay4_tilde *)(w[1]);
    t_sample *in = (t_sample *)(w[2]);
    t_sample *out = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    int maxindex;
    t_word *buf = x->x_vec, *wp;
    int i;

    maxindex = x->x_npoints - 3;
    if(maxindex<0) goto zero;

    if (!buf) goto zero;

    for (i = 0; i < n; i++)
    {
        // advance index with speed input
        x->x_index += *in++;
        //if (x->x_index < 1) x->x_index = x->x_npoints - 3;
        //if (x->x_index > x->x_npoints - 3) x->x_index = 1;
        if (x->x_index < x->x_loop_start + 1) x->x_index = x->x_loop_end - 3;
        if (x->x_index > x->x_loop_end - 3) x->x_index = x->x_loop_start + 1;
        
        double findex = x->x_index;
        
        int index = findex;
        t_sample frac,  a,  b,  c,  d, cminusb;
        static int count;
        if (index < 1)
            index = 1, frac = 0;
        else if (index > maxindex)
            index = maxindex, frac = 1;
        else frac = findex - index;
        wp = buf + index;
        a = wp[-1].w_float;
        b = wp[0].w_float;
        c = wp[1].w_float;
        d = wp[2].w_float;
        cminusb = c-b;
        *out++ = b + frac * (
            cminusb - 0.1666667f * (1.-frac) * (
                (d - a - 3.0f * cminusb) * frac + (d + 2.0f*a - 3.0f*b)
            )
        );
        
    }
    return (w+5);
 zero:
    while (n--) *out++ = 0;

    return (w+5);
}

static void tabplay4_tilde_set(t_tabplay4_tilde *x, t_symbol *s)
{
    t_garray *a;

    x->x_arrayname = s;
    if (!(a = (t_garray *)pd_findbyclass(x->x_arrayname, garray_class)))
    {
        if (*s->s_name)
            pd_error(x, "tabplay4~: %s: no such array", x->x_arrayname->s_name);
        x->x_vec = 0;
    }
    else if (!garray_getfloatwords(a, &x->x_npoints, &x->x_vec))
    {
        pd_error(x, "%s: bad template for tabplay4~", x->x_arrayname->s_name);
        x->x_vec = 0;
    }
    else garray_usedindsp(a);

    printf("got a n points: %d \n", x->x_npoints);
}

static void tabplay4_tilde_set_loop_start(t_tabplay4_tilde *x, t_floatarg ls)
{
    x->x_loop_start = ls;
}


static void tabplay4_tilde_set_loop_end(t_tabplay4_tilde *x, t_floatarg le)
{
    x->x_loop_end = le;
}


static void tabplay4_tilde_dsp(t_tabplay4_tilde *x, t_signal **sp)
{
    tabplay4_tilde_set(x, x->x_arrayname);

    dsp_add(tabplay4_tilde_perform, 4, x,
        sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);

}

static void tabplay4_tilde_free(t_tabplay4_tilde *x)
{
}

void tabplay4_tilde_setup(void)
{
    tabplay4_tilde_class = class_new(gensym("tabplay4~"),
        (t_newmethod)tabplay4_tilde_new, (t_method)tabplay4_tilde_free,
        sizeof(t_tabplay4_tilde), 0, A_DEFSYM, 0);
    CLASS_MAINSIGNALIN(tabplay4_tilde_class, t_tabplay4_tilde, x_speed);
    class_addmethod(tabplay4_tilde_class, (t_method)tabplay4_tilde_dsp, gensym("dsp"), A_CANT, 0);
    class_addmethod(tabplay4_tilde_class, (t_method)tabplay4_tilde_set, gensym("set"), A_SYMBOL, 0);
    class_addmethod(tabplay4_tilde_class, (t_method)tabplay4_tilde_set_loop_start, gensym("loopstart"), A_FLOAT, 0);
    class_addmethod(tabplay4_tilde_class, (t_method)tabplay4_tilde_set_loop_end, gensym("loopend"), A_FLOAT, 0);
}

