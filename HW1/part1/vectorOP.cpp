#include "PPintrin.h"

// implementation of absSerial(), but it is vectorized using PP intrinsics
void absVector(float *values, float *output, int N)
{
  __pp_vec_float x;
  __pp_vec_float result;
  __pp_vec_float zero = _pp_vset_float(0.f);
  __pp_mask maskAll, maskIsNegative, maskIsNotNegative;

  //  Note: Take a careful look at this loop indexing.  This example
  //  code is not guaranteed to work when (N % VECTOR_WIDTH) != 0.
  //  Why is that the case?
  for (int i = 0; i < N; i += VECTOR_WIDTH)
  {

    // All ones
    maskAll = _pp_init_ones();

    // All zeros
    maskIsNegative = _pp_init_ones(0);

    // Load vector of values from contiguous memory addresses
    _pp_vload_float(x, values + i, maskAll); // x = values[i];

    // Set mask according to predicate
    _pp_vlt_float(maskIsNegative, x, zero, maskAll); // if (x < 0) {

    // Execute instruction using mask ("if" clause)
    _pp_vsub_float(result, zero, x, maskIsNegative); //   output[i] = -x;

    // Inverse maskIsNegative to generate "else" mask
    maskIsNotNegative = _pp_mask_not(maskIsNegative); // } else {

    // Execute instruction ("else" clause)
    _pp_vload_float(result, values + i, maskIsNotNegative); //   output[i] = x; }

    // Write results back to memory
    _pp_vstore_float(output + i, result, maskAll);
  }
}

void clampedExpVector(float *values, int *exponents, float *output, int N)
{
  //
  // PP STUDENTS TODO: Implement your vectorized version of
  // clampedExpSerial() here.
  //
  // Your solution should work for any value of
  // N and VECTOR_WIDTH, not just when VECTOR_WIDTH divides N
  //
  __pp_vec_int y;
  __pp_vec_int one_int = _pp_vset_int(1);
  __pp_vec_int zero_int = _pp_vset_int(0);
  __pp_vec_float x;
  __pp_vec_float result;
  __pp_vec_float one_float = _pp_vset_float(1.f);
  __pp_vec_float zero_float = _pp_vset_float(0.f);
  __pp_vec_float upperbound = _pp_vset_float(9.999999f);
  __pp_mask maskAll, maskIsZero, maskIsNotZero, maskGreaterThan;
  for (int i = 0; i < N; i += VECTOR_WIDTH)
  {
    maskAll = (i + VECTOR_WIDTH > N) ? _pp_init_ones(N - i) : _pp_init_ones();
    _pp_vload_float(x, values + i, maskAll);
    _pp_vload_int(y, exponents + i, maskAll);
    _pp_veq_int(maskIsZero, y, zero_int, maskAll);
    maskIsNotZero = _pp_mask_not(maskIsZero);
    _pp_vmove_float(result, one_float, maskIsZero);
    _pp_vmove_float(result, x, maskIsNotZero);

    while(_pp_cntbits(maskIsNotZero) > 0){
      _pp_vsub_int(y, y, one_int, maskIsNotZero);
      _pp_veq_int(maskIsZero, y, zero_int, maskAll);
      maskIsNotZero = _pp_mask_not(maskIsZero);
      maskIsNotZero = _pp_mask_and(maskIsNotZero, maskAll);
      _pp_vmult_float(result, result, x, maskIsNotZero);
    }

    _pp_vgt_float(maskGreaterThan, result, upperbound, maskAll);
    _pp_vmove_float(result, upperbound, maskGreaterThan);
    _pp_vstore_float(output + i, result, maskAll);

  }
}

// returns the sum of all elements in values
// You can assume N is a multiple of VECTOR_WIDTH
// You can assume VECTOR_WIDTH is a power of 2
float arraySumVector(float *values, int N)
{

  //
  // PP STUDENTS TODO: Implement your vectorized version of arraySumSerial here
  //
  __pp_vec_float x;
  __pp_vec_float result = _pp_vset_float(0.f);
  __pp_mask maskAll;

  for (int i = 0; i < N; i += VECTOR_WIDTH)
  {
    maskAll = (i + VECTOR_WIDTH > N) ? _pp_init_ones(N - i): _pp_init_ones();
    _pp_vload_float(x, values + i, maskAll);
    _pp_vadd_float(result, result, x, maskAll);
  }
  int cnt = VECTOR_WIDTH;
  
  while(cnt > 1){
    cnt /= 2;
    _pp_hadd_float(result, result);
    _pp_interleave_float(result, result);
  }
  
  return result.value[0];
}
