//
// Created by anton on 13.02.18.
//

#ifndef MPI_MYDOUBLE_HPP
#define MPI_MYDOUBLE_HPP


#include <cstdint>
#include <vector>
#include <iostream>
#include <algorithm>
#include <functional>
#include <sstream>
#include <iomanip>

template <uint64_t prec>
class MyDouble {
  //positive only
  //subtraction is not implemented
  //normalised double as an argument only

  //less then 2^31 exponent (numbers less then 1e646456993)
  int32_t exp = 0;
  std::vector<uint32_t> signif;
  static const uint64_t exp_mask =    static_cast<const uint64_t>(0x7FF0000000000000LL);
  static const uint64_t signif_mask = static_cast<const uint64_t>(0x000FFFFFFFFFFFFFLL);
  static const int64_t frac_bits = 52;
  static const int64_t exp_bits = 12;
  static const uint64_t normalcoef = 0x7FF/2;

  static void dec_mul(std::vector<uint32_t>& num, uint32_t v);

  static void dec_add(std::vector<uint32_t>& a, const std::vector<uint32_t>& b);

  void for_bit(const std::function<void (int, size_t)> &func) const;

  void shift_right(uint64_t shift, bool add_one = true);

public:

  static MyDouble normalize(uint64_t x, int32_t exp = 0);

  MyDouble(){
    signif.resize(std::max((prec + 31)/32, static_cast<uint64_t>(2)), 0);
  };

  explicit MyDouble(double x){
    if(x == 0.){
      *this = normalize(0);
      return;
    }
    signif.resize(std::max((prec + 31)/32, static_cast<uint64_t>(2)), 0);
    auto p = reinterpret_cast<uint64_t *>(&x);
    exp = static_cast<int32_t>(((*p & exp_mask) >> frac_bits) - normalcoef);
    uint64_t significant = (*p & signif_mask) << exp_bits;
    signif[0] = static_cast <uint32_t>(significant >> 32);
    signif[1] = static_cast <uint32_t>(significant);
  }

  MyDouble(int32_t exp, size_t data_size, uint32_t * data_ptr):exp(exp){
    signif = std::vector<uint32_t>(data_ptr, data_ptr + data_size);
  }

  size_t size(){
    return signif.size();
  }

  int32_t exponent(){
    return exp;
  }

  uint32_t *data(){
    return signif.data();
  }

  friend std::ostream& operator << (std::ostream& stream, MyDouble n){
    return stream << n.exp;
  }

  MyDouble operator + (MyDouble const &val) const;

  void operator += (MyDouble const &val);

  MyDouble operator * (MyDouble const &val) const;

  void operator *= (MyDouble const &val);

  MyDouble operator / (MyDouble const &val) const;

  void operator /= (MyDouble const &val);

  bool operator == (MyDouble const &val) const;

  bool operator != (MyDouble const &val) const {return !(*this == val);}

  bool operator < (MyDouble const &val) const;

  template<uint64_t precV>
  friend MyDouble<precV> average(MyDouble<precV> const &a, MyDouble<precV> const &b);

  std::string bin() const;
  std::string dec() const ;
};


template<uint64_t prec>
std::string MyDouble<prec>::bin() const{
  auto res = std::string();
  res.resize(prec + 2);
  res[0] ='1', res[1] = '.';
  auto f = [&res](int x, size_t ind) mutable {
    res[ind + 2] = static_cast<char>('0' + x);
  };
  for_bit(f);
  res += " * 2^(" + std::to_string(exp)+")";
  return res;
}

template<uint64_t prec>
std::string MyDouble<prec>::dec() const{
  if(*this == normalize(0)){
    return "0";
  }
  std::vector <uint32_t> five{5};
  std::vector <uint32_t> integ{0};
  std::vector <uint32_t> fract{1};
  auto pow = exp;
  for(int i = 0; pow < i - 1; --i){
    dec_mul(fract, 10);
    dec_mul(five, 5);
  }

  if(pow >= 0) {
    integ[0] = 1;
  } else {
    dec_mul(fract, 10);
    dec_add(fract, five);
    dec_mul(five, 5);
  }

  for_bit([&] (int b, size_t ind)mutable{
    if(pow > 0) {
      //computing integer part
      dec_mul(integ, 2);
      if(b)
        dec_add(integ, std::vector<uint32_t>{1});
    } else {
      //computing fraction part
      dec_mul(fract, 10);
      if (b)
        dec_add(fract, five);
      dec_mul(five, 5);
    }
    --pow;
  });
  while(pow  > 0) {
    //finishing computing integer part if not yet
    dec_mul(integ, 2);
    --pow;
  }

  //converting integer part to text
  auto p = integ.data() + integ.size() - 1;
  std::__cxx11::string res = std::to_string(*p);
  std::ostringstream stream;
  for(--p ;p >= integ.data(); --p){
    stream << std::setw(9) << std::setfill('0') << *p;
  }
  res += stream.str();
  stream.str(std::string());

  //converting fractional part
  p = fract.data() + fract.size() - 1;
  stream << *p;
  for(--p ;p >= fract.data(); --p){
    stream << std::setw(9) << std::setfill('0') << *p;;
  }


  std::string frstr = stream.str();
  frstr[0] = '.';

  auto digits = static_cast<unsigned long>(log10l(2) * prec) + 2;
  if(digits <= res.size()) {
    if(digits < res.size()){
      auto len = res.size() - digits;
      res.resize(digits);
      res += "e" + std::to_string(len);
    }
    return res;
  }

  auto len = digits - res.size();
  frstr.resize(len + 1);
  return res + frstr;
}

template<uint64_t prec>
void MyDouble<prec>::dec_mul(std::vector<uint32_t> &num, uint32_t v) {
  //v must be < 10**9
  std::vector<uint32_t> res(num.size(), 0);
  uint32_t carr = 0;
  for(uint i = 0; i < num.size(); ++i){
    uint64_t t = 1ull* v * num[i] + carr;
    carr = static_cast<uint32_t>(t / 1000000000);
    res[i] = static_cast<uint32_t>(t % 1000000000);
  }
  if(carr)
    res.push_back(carr);
  num = res;
}

template<uint64_t prec>
void MyDouble<prec>::dec_add(std::vector<uint32_t> &a,
                             const std::vector<uint32_t> &b) {
  uint32_t carr = 0;
  if(a.size() < b.size()) {
    a.resize(b.size());
  }
  for(uint i = 0; i < a.size(); ++i){
    uint64_t t;
    if(i < b.size())
       t = a[i]  + b[i] + carr;
    else
      t = a[i] + carr;
    carr = static_cast<uint32_t>(t / 1000000000);
    a[i] = static_cast<uint32_t>(t % 1000000000);
  }
  if(carr)
    a.push_back(carr);
}

template<uint64_t prec>
void MyDouble<prec>::for_bit(const std::function<void(int, size_t)> &func) const {
  uint64_t left = prec;
  uint64_t i = 0;
  for (i = 0; i < prec / 32; ++i, left -= 32){
    for(uint j = 0; j < 32; ++j){
      func((signif[i]  & (1ULL << (31 - j))) != 0, i * 32 + j);
    }
  }
  for(uint j = 0; j < left; ++j){
    func((signif[i]  & (1ULL << (31 - j))) != 0, i * 32 + j);
  }
}

template<uint64_t prec>
void MyDouble<prec>::shift_right(uint64_t shift, bool add_one) {
  if(shift == 0)
    return;
  int64_t small = shift % 32;
  int64_t big = shift / 32;
  int64_t s = signif.size();
  if(big >= static_cast<int64_t>(signif.size())) {
    signif = std::vector<uint32_t>(static_cast<unsigned long>(s), 0);
    return;
  }
  int64_t i;
  if (small == 0){
    for(i = s - 1; i >= static_cast<int64_t>(big); --i){
      signif[i] = signif[i - big];
    }
    signif[big - 1] = 1;
    --i;
  }else{
    for(i = s - 1; i >= static_cast<int64_t>(big); --i){
      signif[i] = (signif[i - big] >> small);
      if(i - big  > 0)
        signif[i] += (signif[i - big - 1] << (32ll - small));
    }
    if(add_one)
      signif[big] += 1 << (32 - small);
  }
  for(; i >= 0; --i){
    signif[i] = 0;
  }
}

template<uint64_t prec>
MyDouble<prec> MyDouble<prec>::operator+(MyDouble const &val) const {
  MyDouble<prec> res;
  std::reference_wrapper<const MyDouble<prec>> a = *this, b = val;
  if(b.get().exp > a.get().exp)
    std::swap(a, b);
  int32_t shift = a.get().exp - b.get().exp;

  res.signif = b.get().signif;
  res.exp = a.get().exp;
  if(shift != 0){
    //aligning
    res.shift_right(static_cast<uint64_t>(shift));
  }
  uint64_t carry = 0;
  for(long i =  res.signif.size() - 1; i >= 0; --i){
    uint64_t t = carry + a.get().signif[i] + res.signif[i];
    res.signif[i] = static_cast<uint32_t>(t);
    carry = t >> 32ull;
  }
  if (carry || shift == 0){
    res.exp++;
    //if both occur add 0.1
    res.shift_right(1, (shift == 0) && carry);
  }
  return res;
}

template<uint64_t prec>
void MyDouble<prec>::operator+=(MyDouble const &val) {
  *this  = *this + val;
}

template<uint64_t prec>
MyDouble<prec> MyDouble<prec>::operator*(MyDouble const &val) const {
  MyDouble<prec> res = *this;
  res.exp = exp + val.exp;
  auto e = res.exp;
  for(auto i = static_cast<int>(val.signif.size() - 1); i >= 0; --i) {
    if(val.signif[i] == 0)
      continue;
    //std::cout <<"------" << i << " " << s1.dec() << "\n";
    for(auto j = static_cast<int>(signif.size() - i - 1); j >= 0; --j) {
      uint64_t r = 1ull * val.signif[i] * signif[j];
      if(r == 0)
        continue;
      res += normalize(r, -32*(i + j + 2) + e);
    }
    res += normalize(val.signif[i], -32 *(i + 1) + e);
  }
  return res;
}

template<uint64_t prec>
void MyDouble<prec>::operator*=(MyDouble const &val) {
  *this  = *this * val;
}

template<uint64_t prec>
MyDouble<prec> MyDouble<prec>::normalize(uint64_t x, int32_t exp) {
  MyDouble res;
  res.exp = exp;
  if(x == 0) {
    res.exp = std::numeric_limits<int32_t>::min();
    return res;
  }
  uint64_t upper_bit = 1ull << 63;
  int32_t shift = 1;
  while(!(upper_bit & x)){
    x <<= 1ull;
    ++shift;
  }
  x <<= 1;
  res.exp += 64 - shift;
  res.signif[0] = static_cast<uint32_t>(x >> 32ull);
  if(res.signif.size() > 1)
    res.signif[1] = static_cast<uint32_t>(x);
  return res;
}

template<uint64_t prec>
MyDouble<prec> MyDouble<prec>::operator/(MyDouble const &val) const {
  MyDouble<prec> a = *this, b = val, l(0.5), r(2), m, t;
  a.exp = b.exp = 0;
  m = average(l, r);
  for(uint64_t i = 0; i < prec + 2; ++i){
    t = m * b;
    if (a < t){
      r = m;
    }else{
      l = m;
    }
    m = average(l, r);
  }
  m.exp += exp - val.exp ;
  return m;
}

template<uint64_t prec>
void MyDouble<prec>::operator/=(MyDouble const &val) {
  *this = *this / val;
}

template<uint64_t prec>
bool MyDouble<prec>::operator==(MyDouble const &val) const {
  if(exp != val.exp)
    return false;
  for(uint i = 0; i < signif.size(); ++i){
    if(signif[i] != val.signif[i])
      return false;
  }
  return true;
}

template<uint64_t prec>
bool MyDouble<prec>::operator<(MyDouble const &val) const {
  if(exp > val.exp)
    return false;
  if(exp < val.exp)
    return true;
  for(uint i = 0; i < signif.size(); ++i){
    if(signif[i] > val.signif[i])
      return false;
    if(signif[i] < val.signif[i])
      return true;
  }
  return false;
}

template<uint64_t prec>
MyDouble<prec> average(MyDouble<prec> const &a, MyDouble<prec> const &b) {
  MyDouble<prec> c = a + b;
  c.exp--;
  return c;
}


#endif //MPI_MYDOUBLE_HPP
