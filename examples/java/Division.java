/*@
fixpoint int div(int x, int y) {
  return x / y;
}
@*/

class Division {
  int division_test(int nom, int denom) 
    //@ requires denom != 0;
    //@ ensures result == nom / denom;
  {
    int tmp = nom / denom;
    return tmp;
  }
  
  void division_test2(int nom, int denom) 
    //@ requires denom != 0;
    //@ ensures true;
  {
    int tmp = nom / denom;
    int rest = nom % denom;
    //@ assert denom*tmp + rest == nom;
  }
  
  void bigwiseand_test(int x) 
    //@ requires true;
    //@ ensures true;
  {
    int tmp = x & 255;
    //@ assert 255 >= (x & 255);
  }
  
  int division_test_fail(int nom, int denom) 
    //@ requires true;
    //@ ensures result == nom / denom;
  {
    int tmp = nom / denom; //~ should_fail
    return tmp;
  }
}