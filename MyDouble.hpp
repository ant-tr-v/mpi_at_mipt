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

template <uint64_t pers>
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

  void dec_mul(std::vector<uint32_t>& num, uint32_t v);

  void dec_add(std::vector<uint32_t>& a, const std::vector<uint32_t>& b);

  void for_bit(const std::function<void (int, size_t)> &func);

  void shift_right(uint64_t shift, bool add_one = true);

public:

  MyDouble(){
    signif.resize(std::max((pers + 31)/32, static_cast<uint64_t>(2)), 0);
  };

  explicit MyDouble(double x){
    signif.resize(std::max((pers + 31)/32, static_cast<uint64_t>(2)), 0);
    auto p = reinterpret_cast<uint64_t *>(&x);
    exp = static_cast<int32_t>(((*p & exp_mask) >> frac_bits) - normalcoef);
    uint64_t significant = (*p & signif_mask) << exp_bits;
    signif[0] = static_cast <uint32_t>(significant >> 32);
    signif[1] = static_cast <uint32_t>(significant);
  }

  friend std::ostream& operator << (std::ostream& stream, MyDouble n){
    return stream << n.exp;
  }

  MyDouble operator + (MyDouble &val);

  std::string bin();
  std::string dec();
};


template<uint64_t pers>
std::string MyDouble<pers>::bin() {
  auto res = std::string();
  res.resize(pers + 2);
  res[0] ='1', res[1] = '.';
  auto f = [&res](int x, size_t ind) mutable {
    res[ind + 2] = static_cast<char>('0' + x);
  };
  for_bit(f);
  res += " * 2^(" + std::to_string(exp)+")";
  return res;
}

template<uint64_t pers>
std::string MyDouble<pers>::dec() {
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
    stream << std::setw(9) << std::setfill('0') << *p;
  }


  std::string frstr = stream.str();
  frstr[0] = '.';

  auto digits = static_cast<unsigned long>(log10l(2) * pers) + 2;
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

template<uint64_t pers>
void MyDouble<pers>::dec_mul(std::vector<uint32_t> &num, uint32_t v) {
  //v must be < 10**9
  std::vector<uint32_t> res(num.size(), 0);
  uint32_t carr = 0;
  for(auto i = 0; i < num.size(); ++i){
    uint64_t t = 1ull* v * num[i] + carr;
    carr = static_cast<uint32_t>(t / 1000000000);
    res[i] = static_cast<uint32_t>(t % 1000000000);
  }
  if(carr)
    res.push_back(carr);
  num = res;
}

template<uint64_t pers>
void MyDouble<pers>::dec_add(std::vector<uint32_t> &a,
                             const std::vector<uint32_t> &b) {
  uint32_t carr = 0;
  if(a.size() < b.size()) {
    a.resize(b.size());
  }
  for(auto i = 0; i < a.size(); ++i){
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

template<uint64_t pers>
void MyDouble<pers>::for_bit(const std::function<void(int, size_t)> &func) {
  uint64_t left = pers;
  uint64_t i = 0;
  for (i = 0; i < pers / 32; ++i, left -= 32){
    for(int j = 0; j < 32; ++j){
      func((signif[i]  & (1ULL << (31 - j))) != 0, i * 32 + j);
    }
  }
  for(int j = 0; j < left; ++j){
    func((signif[i]  & (1ULL << (31 - j))) != 0, i * 32 + j);
  }
}

template<uint64_t pers>
MyDouble<pers> MyDouble<pers>::operator+(MyDouble &val) {
  MyDouble<pers> res;
  std::reference_wrapper<MyDouble<pers>> a = *this, b = val;
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
    carry = t >> 32;
  }
  if (carry || shift == 0){
    res.exp++;
    //if both occur add 0.1
    res.shift_right(1, (shift == 0) && carry);
  }
  return res;
}

template<uint64_t pers>
void MyDouble<pers>::shift_right(uint64_t shift, bool add_one) {
  if(shift == 0)
    return;
  auto small = shift % 32;
  auto big = shift / 32;
  auto s = signif.size();
  if(big >= signif.size()) {
    signif = std::vector<uint32_t>(s, 0);
    return;
  }
  int64_t i;
  for(i = s - 1; i >= static_cast<int64_t>(big); --i){
    signif[i] = (signif[i - big] >> small);
    if(i - big  > 0)
      signif[i] += (signif[i - big - 1] << (32 - small));
  }
  if(add_one)
    signif[big] += 1 << (32 - small);
  for(; i >= 0; --i){
    signif[i] = 0;
  }
}


#endif //MPI_MYDOUBLE_HPP
