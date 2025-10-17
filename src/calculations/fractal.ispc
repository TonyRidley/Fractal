// Newton fractal computation for z^n - 1

struct complex
{
    float x;
    float y;
};

static inline varying complex complex_mul(varying complex a, varying complex b)
{
    varying complex result;
    result.x = a.x * b.x - a.y * b.y;
    result.y = a.x * b.y + a.y * b.x;
    return result;
}

static inline varying complex complex_div(varying complex a, varying complex b)
{
    varying float denom = b.x * b.x + b.y * b.y;
    varying complex result;
    result.x = (a.x * b.x + a.y * b.y) / denom;
    result.y = (a.y * b.x - a.x * b.y) / denom;
    return result;
}

static inline varying float complex_abs_sq(varying complex z)
{
    return z.x * z.x + z.y * z.y;
}

static inline varying float complex_abs(varying complex z)
{
    return sqrt(z.x * z.x + z.y * z.y);
}

static inline varying complex complex_pow(varying complex z, uniform int32 n)
{
    if (n == 1)
        return z;
    if (n == 2)
    {
        return complex_mul(z, z);
    }
    if (n == 3)
    {
        varying complex z2 = complex_mul(z, z);
        return complex_mul(z2, z);
    }
    if (n == 4)
    {
        varying complex z2 = complex_mul(z, z);
        return complex_mul(z2, z2);
    }
    if (n == 5)
    {
        varying complex z2 = complex_mul(z, z);
        varying complex z4 = complex_mul(z2, z2);
        return complex_mul(z4, z);
    }

    // For n > 5, use binary exponentiation
    varying complex result;
    result.x = 1.0f;
    result.y = 0.0f;
    varying complex base = z;

    uniform int32 bits = n;
    while (bits > 0)
    {
        if (bits & 1)
            result = complex_mul(result, base);
        base = complex_mul(base, base);
        bits >>= 1;
    }

    return result;
}

static inline varying complex POLY(varying complex z, uniform int32 n)
{
    if (n == 2)
    {
        varying complex z2 = complex_mul(z, z);
        z2.x -= 1.0f;
        return z2;
    }
    if (n == 3)
    {
        varying complex z2 = complex_mul(z, z);
        varying complex z3 = complex_mul(z2, z);
        z3.x -= 1.0f;
        return z3;
    }

    varying complex result = complex_pow(z, n);
    result.x -= 1.0f;
    return result;
}

static inline varying complex DPOLY(varying complex z, uniform int32 n)
{
    if (n == 2)
    {
        varying complex result;
        result.x = 2.0f * z.x;
        result.y = 2.0f * z.y;
        return result;
    }

    varying complex z_pow = complex_pow(z, n - 1);
    varying float fn = (float)n;
    z_pow.x *= fn;
    z_pow.y *= fn;
    return z_pow;
}

// Calculate adaptive post-iterations based on zoom depth
static inline uniform int32 adaptivePostIterations(uniform float zoomLevel)
{
    if (zoomLevel < 10.0f)
        return 3;
    if (zoomLevel < 100.0f)
        return 5;
    if (zoomLevel < 1000.0f)
        return 8;
    return 12;
}

export void compute_newton_fractal_rgba32(
    uniform uint32 pixels[],
    uniform int32 width,
    uniform int32 height,
    uniform int32 stride,
    uniform float x0,
    uniform float y0,
    uniform float x1,
    uniform float y1,
    uniform int32 maxIter,
    uniform int32 n
)
{
    uniform float SQ3 = 1.7320508075688772f;
    uniform float sx = (x1 - x0) / (float)width;
    uniform float sy = (y1 - y0) / (float)height;

    // Calculate zoom level
    uniform float defaultRange = 10.0f;
    uniform float currentRange = x1 - x0;
    uniform float zoomLevel = defaultRange / currentRange;

    // Adaptive post-iterations based on zoom depth
    uniform int32 postIterations = adaptivePostIterations(zoomLevel);

    // Precompute thresholds
    uniform float convergence_threshold = 0.0001f;
    uniform float derivative_threshold = 1e-20f;
    uniform float decay = 0.95f;

    // Precompute color constants
    uniform float SQ3_024 = SQ3 * 0.24f;
    uniform float f_024 = 0.24f;
    uniform float f_048 = 0.48f;

    foreach (py = 0 ... height, px = 0 ... width)
    {
        varying float cx = x0 + (float)px * sx;
        varying float cy = y0 + (float)py * sy;

        varying complex z;
        z.x = cx;
        z.y = -cy;

        varying complex w = POLY(z, n);

        varying float f = 1.0f;
        varying int32 iteration = 0;

        // Main Newton iteration with early convergence detection
        for (uniform int32 iter = 0; iter < maxIter; ++iter)
        {
            if (f <= 0.01f || complex_abs_sq(w) <= convergence_threshold)
                break;

            varying complex dpoly = DPOLY(z, n);
            if (complex_abs_sq(dpoly) < derivative_threshold)
                break;

            varying complex step = complex_div(w, dpoly);
            z.x -= step.x;
            z.y -= step.y;
            f *= decay;
            w = POLY(z, n);

            iteration++;
        }

        varying complex dpoly = DPOLY(z, n);
        if (complex_abs_sq(dpoly) > derivative_threshold)
        {
            varying complex step = complex_div(w, dpoly);
            z.x -= step.x;
            z.y -= step.y;
        }

        // Adaptive post-iterations based on convergence speed
        uniform int32 postIters = postIterations;
        if (iteration < 3)
        {
            postIters = max(1, postIterations / 3);
        }
        else if (iteration < 10)
        {
            postIters = max(2, postIterations / 2);
        }

        for (uniform int32 iter = 0; iter < postIters; ++iter)
        {
            dpoly = DPOLY(z, n);
            if (complex_abs_sq(dpoly) < derivative_threshold)
                break;
            varying complex p = POLY(z, n);
            varying complex step = complex_div(p, dpoly);
            z.x -= step.x;
            z.y -= step.y;
        }

        // Color calculation
        varying complex z2 = complex_mul(z, z);
        varying float conj_dot = z.x * z.x + z.y * z.y;
        z2.x = f * z2.x / conj_dot;
        z2.y = f * z2.y / conj_dot;

        varying float gr0 = 0.5f * f + f_024 * z2.x - SQ3_024 * z2.y;
        varying float gr1 = 0.5f * f + f_024 * z2.x + SQ3_024 * z2.y;
        varying float gr2 = 0.5f * f - f_048 * z2.x;

        // Clamp and convert to uint8
        gr0 = clamp(gr0, 0.0f, 1.0f);
        gr1 = clamp(gr1, 0.0f, 1.0f);
        gr2 = clamp(gr2, 0.0f, 1.0f);

        varying uint32 r = (uint32)(gr0 * 255.0f);
        varying uint32 g = (uint32)(gr1 * 255.0f);
        varying uint32 b = (uint32)(gr2 * 255.0f);

        // Pack RGBA into uint32
        varying uint32 color = 0xFF000000u | (b << 16) | (g << 8) | r;

        pixels[py * stride + px] = color;
    }
}