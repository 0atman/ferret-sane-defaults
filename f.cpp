// ferret-lisp build:c28bb1f

        #ifndef FERRET_RUNTIME_H
        #define FERRET_RUNTIME_H

         // Literals
         namespace ferret{
           constexpr auto operator "" _MB( unsigned long long const x ) -> long { return 1024L * 1024L * (long)x; }
           constexpr auto operator "" _KB( unsigned long long const x ) -> long { return 1024L * (long)x; }
         }

         // Detect Hardware
         # define FERRET_CONFIG_SAFE_MODE TRUE

         #if !defined(FERRET_SAFE_MODE)
           #if defined(__APPLE__) ||                       \
             defined(_WIN32) ||                            \
             defined(__linux__) ||                         \
             defined(__unix__) ||                          \
             defined(_POSIX_VERSION)
           
             # undef  FERRET_CONFIG_SAFE_MODE
             # define FERRET_STD_LIB TRUE
           #endif
           
           #if defined(ARDUINO)

             # define FERRET_HARDWARE_ARDUINO TRUE

             #if !defined(FERRET_HARDWARE_ARDUINO_UART_PORT)
               # define FERRET_HARDWARE_ARDUINO_UART_PORT Serial
             #endif
           #endif
           
           #if defined(FERRET_HARDWARE_ARDUINO)
             # undef  FERRET_CONFIG_SAFE_MODE
             # define FERRET_DISABLE_STD_MAIN TRUE


             #if defined(__AVR__)
               # undef  FERRET_MEMORY_POOL_PAGE_TYPE
               # define FERRET_MEMORY_POOL_PAGE_TYPE uint8_t
             #endif

           #endif
         #endif

         #if defined(FERRET_CONFIG_SAFE_MODE)
           # define FERRET_DISABLE_MULTI_THREADING TRUE
           # define FERRET_DISABLE_STD_OUT TRUE
         #endif
         #ifdef FERRET_STD_LIB
          #include <iostream>
          #include <iomanip>
          #include <sstream>
          #include <cstdio>
          #include <cstdlib>
          #include <cstddef>
          #include <cmath>
          #include <vector>
          #include <algorithm>
          #include <chrono>
          #include <atomic>
          #include <mutex>
          #include <thread>
          #include <future>
         #endif

         #ifdef FERRET_HARDWARE_ARDUINO
          #include <Arduino.h>
          #include <stdio.h>
          #include <stdlib.h>
          #include <stdint.h>
         #endif

         #ifdef FERRET_CONFIG_SAFE_MODE
          #include <stdio.h>
          #include <stdlib.h>
          #include <stdint.h>
          #include <math.h>
         #endif

         namespace ferret {
         #if defined(FERRET_DISABLE_MULTI_THREADING)
           class mutex {
           public:
             void lock()   {} 
             void unlock() {} 
           };
         #else
           #if defined(FERRET_STD_LIB)
             class mutex {
               ::std::mutex m;
             public:
               void lock()   { m.lock();   } 
               void unlock() { m.unlock(); }
             };
           #endif

           #if defined(FERRET_HARDWARE_ARDUINO)
             class mutex {
             public:
               void lock()   { noInterrupts(); } 
               void unlock() { interrupts();   }
             };
           #endif
         #endif
         }

         namespace ferret {
           class lock_guard{
             mutex & _ref;
           public:
             explicit lock_guard(const lock_guard &) = delete;
             explicit lock_guard(mutex & mutex) : _ref(mutex) { _ref.lock(); };
             ~lock_guard() { _ref.unlock(); }
           };
         }

         // Number Configuration
         namespace ferret{
         #if !defined(FERRET_NUMBER_TYPE)
            #define FERRET_NUMBER_TYPE int
         #endif

         #if !defined(FERRET_REAL_TYPE)
            #define FERRET_REAL_TYPE   double
         #endif

         #if !defined(FERRET_REAL_EPSILON)
            #define FERRET_REAL_EPSILON   0.00001
         #endif
           
           typedef FERRET_NUMBER_TYPE           number_t;                   // Whole number Container.
           typedef FERRET_REAL_TYPE             real_t;                     // Real number Container.
           const   real_t                       real_epsilon(FERRET_REAL_EPSILON);
         #if !defined(FERRET_DISABLE_STD_OUT)
           const   size_t                       number_precision = 4;       // number Format String (fprinf)
         #endif
         }
         namespace ferret{
           constexpr auto operator "" _pi(long double x) -> double {
             return 3.14159265358979323846 * (double)x;
           }

           constexpr auto operator "" _pi(unsigned long long int  x) -> double {
             return 1.0_pi * (double)x;
           }

           constexpr auto operator "" _deg(long double x) -> double {
             return (1.0_pi * (double)x) / 180;
           }

           constexpr auto operator "" _deg(unsigned long long int  x) -> double {
             return 1.0_deg * (double)x;
           }
         }

         namespace ferret{
           // Fixed Point Real
           #if !defined(__clang__)
           constexpr auto operator "" _QN(long double x) -> int {
             return (int)::floor(::log(1.0/(double)x)/::log(2));
           }
           #endif
           
           template<int bits> struct fixed_real_container;
           template<> struct fixed_real_container<8>  { typedef int8_t  base_type;
                                                        typedef int16_t next_type; };
           template<> struct fixed_real_container<16> { typedef int16_t base_type;
                                                        typedef int32_t next_type; };
           template<> struct fixed_real_container<32> { typedef int32_t base_type;
                                                        typedef int64_t next_type; };
           template<> struct fixed_real_container<64> { typedef int64_t base_type;
                                                        typedef int64_t next_type; };
           
           template<int bits, int exp>
           class fixed_real{
             typedef fixed_real fixed;
             typedef typename fixed_real_container<bits>::base_type base;
             typedef typename fixed_real_container<bits>::next_type next;
           
             base m;
             static const int N      = (exp - 1);
             static const int factor = 1 << N;
           
             template<typename T>
             inline base from(T d) const { return (base)(d * factor); }
           
             template<typename T>
             inline T to_rational() const { return T(m) / factor; }
           
             template<typename T>
             inline T to_whole() const { return (T)(m >> N); }
               
           public:
           
             //from types
             explicit fixed_real( )           : m(0) { }
             template<typename T>
             explicit fixed_real(T v)         : m(from<T>(v)) {}
           
             template<typename T>
             fixed& operator=(T v)        { m = from<T>(v); return *this; }
               
             //to types
             template<typename T>
             operator T()           const { return to_whole<T>();    }
             operator double()      const { return to_rational<double>(); }
               
             // operations
             fixed& operator+= (const fixed& x) { m += x.m; return *this; }
             fixed& operator-= (const fixed& x) { m -= x.m; return *this; }
             fixed& operator*= (const fixed& x) { m = (base)(((next)m * (next)x.m) >> N); return *this; }
             fixed& operator/= (const fixed& x) { m = (base)(((next)m * factor) / x.m); return *this; }
             fixed& operator*= (int x)          { m *= x; return *this; }
             fixed& operator/= (int x)          { m /= x; return *this; }
             fixed  operator-  ( )              { return fixed(-m); }
               
             // friend functions
             friend fixed operator+ (fixed x, const fixed& y) { return x += y; }
             friend fixed operator- (fixed x, const fixed& y) { return x -= y; }
             friend fixed operator* (fixed x, const fixed& y) { return x *= y; }
             friend fixed operator/ (fixed x, const fixed& y) { return x /= y; }
               
             // comparison operators
             friend bool operator== (const fixed& x, const fixed& y) { return x.m == y.m; }
             friend bool operator!= (const fixed& x, const fixed& y) { return x.m != y.m; }
             friend bool operator>  (const fixed& x, const fixed& y) { return x.m > y.m; }
             friend bool operator<  (const fixed& x, const fixed& y) { return x.m < y.m; }
             friend bool operator>= (const fixed& x, const fixed& y) { return x.m >= y.m; }
             friend bool operator<= (const fixed& x, const fixed& y) { return x.m <= y.m; }
           
           #if defined(FERRET_STD_LIB)
             friend std::ostream& operator<< (std::ostream& stream, const fixed& x) {
               stream << (double)x;
               return stream;
             }
           #endif
           };
           namespace euclidean{
             template <size_t D, typename T>
             struct vector {
               T d[D];
            
               T& operator [](size_t idx)      { return d[idx]; }
               T operator [](size_t idx) const { return d[idx]; }
               
               friend vector operator+ (vector u, const vector& v) {
                 vector result;
                 for (size_t i = 0; i < D; i++)
                   result.d[i] = u.d[i] + v.d[i];
                 return result;
               }
            
               friend vector operator- (vector u, const vector& v) {
                 vector result;
                 for (size_t i = 0; i < D; i++)
                   result.d[i] = u.d[i] - v.d[i];
                 return result;
               }
            
               friend vector operator* (vector u, const T& v) {
                 vector result;
                 for (size_t i = 0; i < D; i++)
                   result.d[i] = u.d[i] * v;
                 return result;
               }
            
               friend bool operator== (const vector& x, const vector& y) {
                 for (size_t i = 0; i < D; i++)
                   if (x.d[i] != y.d[i])
                     return false;
                 return true;
               }
            
               T magnitude(){
                 T acc = 0;
                 for(size_t i = 0; i < D; i++){
                   T t = d[i] * d[i];
                   acc += t;
                 }
            
                 return sqrt(acc);
               }
            
               vector normalize(){
                 T mag = magnitude();
                 if (mag == 0)
                   return vector{{0}};
            
                 vector r;
                 for(size_t i = 0; i < D; i++){
                   r[i] = d[i] / mag;
                 }
                 
                 return r;
               }
            
               T dist(vector v){ return ((*this) - v).magnitude(); }
            
           #if defined(AUTOMATON_STD_LIB)
               friend std::ostream& operator<< (std::ostream& stream, const vector& x) {
                 stream << '[';
                 for (size_t i = 0; i < D - 1; i++)
                   stream << x.d[i] << ',';
                 stream << x.d[D - 1] << ']';
                 return stream;
               }
           #endif
             };
           
             typedef euclidean::vector<2, ferret::real_t> vector_2d;
             typedef euclidean::vector<3, ferret::real_t> vector_3d;
           }
           // Containers
           enum class byte : unsigned char { };
           template<class Type>
           struct array_seq {
             Type* p;
             explicit array_seq(Type* p) : p(p) {}
             bool operator!=(array_seq rhs) {return p != rhs.p;}
             Type& operator*() {return *p;}
             void operator++() {++p;}
           };
            
           template<class T, size_t S>
           struct array {
             T data[S];
                 
             T& operator [](int idx)      { return data[idx]; }
             T operator [](int idx) const { return data[idx]; }
               
             array_seq<T> begin() { return array_seq<T>(data);      }
             array_seq<T> end()   { return array_seq<T>(data + S);  }
             size_t       size()  { return S;                       }
           };
           template<size_t S>
           class bitset {
           private:
             byte bits[S / 8 + 1];
                      
             inline size_t index (size_t i) { return i / 8; }
             inline size_t offset(size_t i) { return i % 8; }
                        
           public:
                        
             bitset() : bits{ (byte)0x00 } { }
                      
             inline void set   (size_t b){
               bits[index(b)] = (byte)((int)bits[index(b)] |  (1 << (offset(b))));
             }
                        
             inline void reset (size_t b){
               bits[index(b)] = (byte)((int)bits[index(b)] & ~(1 << (offset(b))));
             }
                        
             inline bool test  (size_t b){
               return ((int)bits[index(b)] & (1 << (offset(b))));
             }
           };
         }

         // Initialize Hardware
         namespace ferret{
           #if !defined(FERRET_UART_RATE)
             # define FERRET_UART_RATE 9600
           #endif
           #if !defined(FERRET_IO_STREAM_SIZE)
             # define FERRET_IO_STREAM_SIZE 80
           #endif
           #if defined(FERRET_DISABLE_STD_OUT)
              namespace runtime{
                void init(){ }
               
                template <typename T>
                void print(T){ }
              }
           #endif
           #if defined(FERRET_STD_LIB) && !defined(FERRET_DISABLE_STD_OUT)
             namespace runtime{
               void init(){}
               
               template <typename T>
               void print(const T t){ std::cout << t; }
           
               template <>
               void print(const real_t n){
                 std::cout << std::fixed << std::setprecision(number_precision) << n;
               }
           
               void read_line(char *buff, std::streamsize len){
                 std::cin.getline(buff, len);
               }
             }
           #endif
           #if defined(FERRET_HARDWARE_ARDUINO) && !defined(FERRET_DISABLE_STD_OUT) 
             namespace runtime{
               void init(){ FERRET_HARDWARE_ARDUINO_UART_PORT.begin(FERRET_UART_RATE); }
           
               template <typename T>
               void print(const T t){ FERRET_HARDWARE_ARDUINO_UART_PORT.print(t); }
           
               template <>
               void print(const real_t d){ FERRET_HARDWARE_ARDUINO_UART_PORT.print(double(d)); }
               
               template <>
               void print(void *p){
                 FERRET_HARDWARE_ARDUINO_UART_PORT.print((size_t)p,HEX);
               }
           
               template <> void print(const void * const p){
                 FERRET_HARDWARE_ARDUINO_UART_PORT.print((size_t)p, HEX);
               }
           
               void read_line(char *buff, size_t len){
                 uint8_t idx = 0;
                 char c;
                 do{
                   while (FERRET_HARDWARE_ARDUINO_UART_PORT.available() == 0);
                   c = FERRET_HARDWARE_ARDUINO_UART_PORT.read();
                   buff[idx++] = c;
                 }while (c != '\n');
                 buff[--idx] = 0x00;
               }
              }
           #endif
         }

         // Object System Base
         namespace ferret{
           namespace memory{
             inline size_t align_of(uintptr_t size, size_t align){
               return (size + align - 1) & ~(align - 1);
             }
           
             template<class T>
             size_t align_of(const void * ptr) {
               return align_of(reinterpret_cast<uintptr_t>(ptr), sizeof(T));
             }
               
             inline size_t align_req(uintptr_t size, size_t align){
               size_t adjust = align - (size & (align - 1));
                 
               if(adjust == align)
                 return 0;
               return adjust;
             }
           
             template<class T>
             size_t align_req(const void * ptr) {
               return align_req(reinterpret_cast<uintptr_t>(ptr), sizeof(T));
             }
           }
           #ifdef FERRET_MEMORY_POOL_SIZE
           namespace memory{
             namespace allocator{
               template<typename page_size, size_t pool_size>
               class memory_pool{
               public:
                 bitset<pool_size> used;
                 page_size pool[pool_size];
                 size_t offset;
                 size_t page_not_found;
           
                 memory_pool() : pool{0}, offset(0), page_not_found(pool_size + 1) { }
           
                 inline size_t chunk_length(size_t size){
                   size_t d = (size / sizeof(page_size));
                   size_t f = (size % sizeof(page_size));
           
                   if (f == 0)
                     return d;
                   else
                     return (d + 1);
                 }
           
                 inline bool chunk_usable(size_t begin, size_t end){
                   for(size_t i=begin; i < end; i++)
                     if (used.test(i))
                       return false;
                   return true;
                 }
           
                 inline size_t next_page(size_t begin){
                   for(size_t i=begin; i < pool_size; i++)
                     if (!used.test(i))
                       return i;
                   return pool_size;
                 }
           
                 inline size_t scan_pool(size_t pages_needed, size_t offset = 0){
                   for(;;){
                     size_t begin = next_page(offset);
                     size_t end   = begin + pages_needed;
             
                     if (end > pool_size)
                       return page_not_found;
                   
                     if (chunk_usable(begin, end))
                       return begin;
             
                     offset = end;
                   }
                 }
           
                 void *allocate(size_t req_size){
                   size_t length = chunk_length(++req_size);
                   size_t page   = scan_pool(length, offset);
           
                   if (page == page_not_found){
                     page = scan_pool(length);
                     if (page == page_not_found)
                       return nullptr;
                   }
                   
                   pool[page] = length;
                   offset = page + length;
                   for(size_t i = page; i < offset; i++)
                     used.set(i);
           
                   return &pool[++page];
                 }
           
                 void free(void *p){
                   ptrdiff_t begin = (static_cast<page_size *>(p) - pool) - 1;
                   ptrdiff_t end = begin + (ptrdiff_t)pool[begin];
           
                   for (ptrdiff_t i = begin; i < end; i++)
                     used.reset((size_t)i);
                 }
               };
             }
           }
           #endif
           #if defined(FERRET_MEMORY_POOL_SIZE) && !defined(FERRET_ALLOCATOR)
           
            #define FERRET_ALLOCATOR memory::allocator::pool
           
            #if !defined(FERRET_MEMORY_POOL_PAGE_TYPE)
             #define FERRET_MEMORY_POOL_PAGE_TYPE size_t
             #define FERRET_MEMORY_POOL_PAGE_COUNT                                   \
               (FERRET_MEMORY_POOL_SIZE / sizeof(FERRET_MEMORY_POOL_PAGE_TYPE))
            #else
             #define FERRET_MEMORY_POOL_PAGE_COUNT FERRET_MEMORY_POOL_SIZE
            #endif
           
           namespace memory{
             namespace allocator{
           
               memory_pool<FERRET_MEMORY_POOL_PAGE_TYPE, FERRET_MEMORY_POOL_PAGE_COUNT> program_memory;
           
               class pool{
               public:
           
                 static void init(){ }
                 
                 template<typename FT>
                 static inline void*  allocate(){ return program_memory.allocate(sizeof(FT)); }
                 
                 static inline void   free(void * ptr){ program_memory.free(ptr); }
               };
             }
           }
           #endif
           #ifdef FERRET_MEMORY_BOEHM_GC
           
           #define FERRET_ALLOCATOR memory::allocator::gc
           #define FERRET_DISABLE_RC true
           
           #include <gc.h>
           
           namespace memory{
             namespace allocator{
               
               class gc{
               public:
           
                 static void init(){ GC_INIT(); }
                 
                 template<typename FT>
                 static inline void* allocate(){
           #ifdef FERRET_DISABLE_MULTI_THREADING
                   return GC_MALLOC(sizeof(FT));
           #else
                   return GC_MALLOC_ATOMIC(sizeof(FT));
           #endif
                 }
               
                 static inline void  free(void * ptr){ }
               };
             }
           }
           #endif
           #if !defined(FERRET_ALLOCATOR)
           
           #define FERRET_ALLOCATOR memory::allocator::system
           
           namespace memory{
             namespace allocator{
           
               class system{
               public:
           
                 static void init(){ }
           
                 template<typename FT>
                 static inline void* allocate(){ return ::malloc(sizeof(FT)); }
           
                 static inline void  free(void * ptr){ ::free(ptr); } 
               };
             }
           }
           #endif
           namespace memory{
             namespace allocator{
               class synchronized{
                 static mutex lock;
               public:
           
                 static void init(){ FERRET_ALLOCATOR::init(); }
           
                 template<typename FT>
                 static inline void* allocate(){
                   lock_guard guard(lock);
                   return FERRET_ALLOCATOR::allocate<FT>();
                 }
           
                 static inline void  free(void * ptr){
                   lock_guard guard(lock);
                   FERRET_ALLOCATOR::free(ptr);
                 }
               };
             }
           }
           #if  !defined(FERRET_DISABLE_MULTI_THREADING)
           
             #if defined(FERRET_MEMORY_POOL_SIZE) || defined(FERRET_HARDWARE_ARDUINO)
               mutex memory::allocator::synchronized::lock;
               #undef  FERRET_ALLOCATOR
               #define FERRET_ALLOCATOR memory::allocator::synchronized
             #endif
           
           #endif
           #if !defined(FERRET_RC_POLICY)
           namespace memory {
             namespace gc {
           
           #if !defined(FERRET_RC_TYPE)
             #define FERRET_RC_TYPE unsigned int
           #endif
               
           #if defined(FERRET_DISABLE_RC)
           
           #define FERRET_RC_POLICY memory::gc::no_rc
               
               class no_rc{
               public:
           
                 inline void inc_ref() { }
                 inline bool dec_ref() { return false; }
               };
           
           #else
           
               template<typename T>
               class rc{
               public:
                 rc() : ref_count(0) {}
           
                 inline void inc_ref() { ref_count++; }
                 inline bool dec_ref() { return (--ref_count == 0); }
               
               private:
                 T ref_count;
               };    
           
               #if defined(FERRET_DISABLE_MULTI_THREADING) || !defined(FERRET_STD_LIB)
                 #define FERRET_RC_POLICY memory::gc::rc<FERRET_RC_TYPE>
               #endif
               
               #if defined(FERRET_STD_LIB) && !defined(FERRET_DISABLE_MULTI_THREADING)
                 #define FERRET_RC_POLICY memory::gc::rc<::std::atomic<FERRET_RC_TYPE>>
               #endif
           #endif
             }
           }
           #endif
           class var;
           class seekable_i;
           
           template <typename rc>
           class object_i : public rc{
           public:
             object_i() { }
             virtual ~object_i() { };
             
             virtual size_t type() const = 0;
             
           #if !defined(FERRET_DISABLE_STD_OUT)
             virtual void stream_console() const = 0;
           #endif
             
             virtual bool equals(var const & o) const = 0;
           
             virtual seekable_i* cast_seekable_i() { return nullptr; }
           
             void* operator new(size_t, void* ptr){ return ptr; }
             void  operator delete(void * ptr){ FERRET_ALLOCATOR::free(ptr); }
           };
           
           typedef object_i<FERRET_RC_POLICY> object;
           class var{
           public:
             explicit var(object* o = nullptr) : obj(o) { inc_ref(); }
           
             var(const var& o) : obj(o.obj) { inc_ref(); }
             var(var&& o) : obj(o.obj) { o.obj = nullptr; }
               
             ~var() { dec_ref(); }
           
             var& operator=(var&& other){
               if (this != &other){
                 dec_ref();
                 obj = other.obj;
                 other.obj = nullptr;
               }
               return *this;
             }
             
             var& operator= (const var& other){
               if (obj != other.obj){
                 dec_ref();
                 obj = other.obj;
                 inc_ref();
               }
               return *this;
             }
           
             bool equals (var const & rhs) const;
           
             bool operator==(const var& other) const { return equals(other); }
           
             bool operator!=(const var& other) const { return !equals(other); }
             
             operator bool() const;
           
           #if !defined(FERRET_DISABLE_STD_OUT)
             void stream_console() const {
               if (obj != nullptr )
                 obj->stream_console();
               else
                 runtime::print("nil");
             }
           #endif
                 
             inline object* get() const { return obj; }
             
             template<typename T>
             inline T* cast() const { return static_cast<T*>(obj); }
           
             inline bool is_type(size_t type) const { 
               return (static_cast<object*>(obj)->type() == type);
             }
           
             inline bool is_nil() const { return (obj == nullptr); }
           
           private:
             inline void inc_ref(){
           #if !defined(FERRET_DISABLE_RC)
               // Only change if non-null
               if (obj) obj->inc_ref();
           #endif
             }
               
             inline void dec_ref(){
           #if !defined(FERRET_DISABLE_RC)
               // Only change if non-null
               if (obj){
                 // Subtract and test if this was the last pointer.
                 if (obj->dec_ref()){
                   delete obj;
                   obj = nullptr;
                 }
               }
           #endif
             }
               
             object* obj;
           };
           
           template<>
           inline seekable_i* var::cast<seekable_i>() const { return obj->cast_seekable_i(); }
           template<typename FT, typename... Args>
           inline var obj(Args... args) {
             void * storage = FERRET_ALLOCATOR::allocate<FT>();
             return var(new(storage) FT(args...));
           }
           
           inline var nil(){
             return var();
           }
         }

         // Runtime Prototypes
         namespace ferret{
           namespace runtime {
             var list(var const & v);
             var list(var const & v);
             template <typename... Args>
             var list(var const & first, Args const & ... args);
           
             var first(var const & coll);
             var rest(var const & coll);
             var cons(var const & x, var const & seq);
             var nth(var const & seq, number_t index);
             var nthrest(var const & seq, number_t index);
             size_t count(var const & seq);
             bool is_seqable(var const & seq);
           }
           template<typename T, typename... Args>
           inline var run(T const & fn, Args const & ... args);
                 
           template<typename T>
           inline var run(T const & fn);
           
           template<>
           inline var run(var const &);
           namespace runtime{
             #undef min
             #undef abs
           
             template<typename T>
             constexpr T min(T a, T b){
               return ((a) < (b) ? (a) : (b));
             }
           
             template<typename T>
             constexpr T abs(T a){
               return ((a) < (T)0 ? -(a) : (a));
             }
           }
         }
        #endif

        // Objects
        namespace ferret{
         #ifndef FERRET_OBJECT_SEEKABLE_I
               #define FERRET_OBJECT_SEEKABLE_I

               class seekable_i {
               public:

                 virtual var cons(var const & x) = 0;
                 virtual var first() const = 0;
                 virtual var rest() const = 0;
               };
               #endif
         #ifndef FERRET_OBJECT_LAMBDA_I
               #define FERRET_OBJECT_LAMBDA_I
                 namespace runtime {
                   namespace type {
                      const size_t lambda_i = 3949;}}

               class lambda_i : public object {
                public:
                 virtual var invoke(var const & args) const = 0;

                 size_t type() const { return runtime::type::lambda_i; }

                 bool equals(var const & o) const {
                   return (this == o.get());
                 }

               #if !defined(FERRET_DISABLE_STD_OUT)
                 void stream_console() const {
                   runtime::print("lambda");
                 }
               #endif
               };
               #endif
         #ifndef FERRET_OBJECT_DEREF_I
               #define FERRET_OBJECT_DEREF_I

               class deref_i : public object {
                public:

                 virtual var deref() = 0;
               };
               #endif
         #ifndef FERRET_OBJECT_BOOLEAN
               #define FERRET_OBJECT_BOOLEAN
                 namespace runtime {
                   namespace type {
                      const size_t boolean = 3951;}}

               class boolean final : public object {
                 const bool value;
               public:

                 size_t type() const final { return runtime::type::boolean; }

                 bool equals(var const & o) const final {
                   return (value == o.cast<boolean>()->container());
                 }

               #if !defined(FERRET_DISABLE_STD_OUT)
                 void stream_console() const final {
                   if (value)
                     runtime::print("true");
                   else
                     runtime::print("false");
                 }
               #endif

                 explicit boolean(bool b) : value(b) {} 

                 bool container() const {
                   return value;
                 }
               };

               namespace cached{
                 const var true_t = obj<ferret::boolean>(true);
                 const var false_t = obj<ferret::boolean>(false);
               }

               var::operator bool() const {
                 if (obj == nullptr)
                   return false;
                 else if (obj->type() == runtime::type::boolean)
                   return static_cast<boolean*>(obj)->container();
                 else
                   return true;
               }

               bool var::equals (var const & other) const {
                 if ( ( is_nil() && !other.is_nil()) ||
                      (!is_nil() &&  other.is_nil()))
                   return false;

                 if (get() == other.get())
                   return true;
                 
                 if (runtime::is_seqable(*this) && runtime::is_seqable(other))
                   return get()->equals(other);
                 else if (obj->type() != other.cast<object>()->type())
                   return false;
                 else
                   return get()->equals(other);
               }
               #endif
         #ifndef FERRET_OBJECT_POINTER
               #define FERRET_OBJECT_POINTER
                 namespace runtime {
                   namespace type {
                      const size_t pointer = 3952;}}

               class pointer final : public object {
                 void * _payload;
               public:


                 size_t type() const final { return runtime::type::pointer; }

                 bool equals(var const & o) const final {
                   return (_payload == o.cast<pointer>()->payload());
                 }

               #if !defined(FERRET_DISABLE_STD_OUT)
                 void stream_console() const final {
                   runtime::print("pointer<");
                   runtime::print(_payload);
                   runtime::print(">");
                 }
               #endif

                 explicit pointer(void* p) : _payload(p) {} 

                 void* payload() const {
                   return _payload;
                 }
                 template<typename T> static T* to_pointer(var const & v){
                   return ((T *) v.cast<pointer>()->payload());
                 }
                 template<typename T> static T& to_reference(var const & v){
                   return (*(pointer::to_pointer<T>(v)));
                 }
               };
               #endif
         #ifndef FERRET_OBJECT_VALUE
               #define FERRET_OBJECT_VALUE
                 namespace runtime {
                   namespace type {
                      const size_t value = 3953;}}

               template <typename T>
               class value final : public object {
                 T _value;
                public:

                 size_t type() const final { return runtime::type::value; }

                 bool equals(var const & o) const final {
                   return (this == o.get());
                 }

               #if !defined(FERRET_DISABLE_STD_OUT)
                 void stream_console() const final {
                   runtime::print("value<");
                   const void* addr = this;
                   runtime::print(addr);
                   runtime::print(">");
                 }
               #endif

                 template <typename... Args>
                 explicit value(Args&&... args) : _value(static_cast<Args&&>(args)...) { } 

                 T to_value() const {
                   return _value;
                 }
                 
                 static T to_value(var const & v){
                   return v.cast<value<T>>()->to_value();
                 }

                 T & to_reference() {
                   return _value;
                 }
                 
                 static T & to_reference(var const & v) {
                   return v.cast<value<T>>()->to_reference();
                 }  
               };
               #endif
         #ifndef FERRET_OBJECT_NUMBER
               #define FERRET_OBJECT_NUMBER
                 namespace runtime {
                   namespace type {
                      const size_t number = 3954;}}

               class number final : public object {
                 const real_t _word;
               public:


                 size_t type() const final { return runtime::type::number; }

                 bool equals(var const & o) const final {
                   if (runtime::abs(_word - o.cast<number>()->word()) < real_epsilon)
                     return true;
                   else
                     return false;
                 }

               #if !defined(FERRET_DISABLE_STD_OUT)
                 void stream_console() const final {
                   runtime::print(_word);
                 }
               #endif

                 template<typename T> explicit number(T x) : _word((real_t)x) {} 

                 real_t word() const {
                   return _word;
                 }
                 
                 template<typename T> T as() const {
                   T::unimplemented_function;
                 }
                 
                 var add(var const & v) const {
                   return obj<number>(_word + v.cast<number>()->word());
                 }
                 var sub(var const & v) const {
                   return obj<number>(_word - v.cast<number>()->word());
                 }
                 var mul(var const & v) const {
                   return obj<number>(_word * v.cast<number>()->word());
                 }
                 var div(var const & v) const {
                   return obj<number>(_word / v.cast<number>()->word());
                 }
                 var is_smaller(var const & v) const {
                   return obj<boolean>(_word < v.cast<number>()->word());
                 }
                 var is_smaller_equal(var const & v) const {
                   return obj<boolean>(_word <= v.cast<number>()->word());
                 }
                 var is_bigger(var const & v) const {
                   return obj<boolean>(_word > v.cast<number>()->word());
                 }
                 var is_bigger_equal(var const & v) const {
                   return obj<boolean>(_word >= v.cast<number>()->word());
                 }
                 
                 template<typename T> static T to(var const & v){
                   return (T)v.cast<number>()->word();
                 }
               };
               #endif
         #ifndef FERRET_OBJECT_EMPTY_SEQUENCE
               #define FERRET_OBJECT_EMPTY_SEQUENCE
                 namespace runtime {
                   namespace type {
                      const size_t empty_sequence = 3955;}}

               class empty_sequence final : public object {
               public:

                 size_t type() const final { return runtime::type::empty_sequence; }

                 bool equals(var const & ) const final {
                   return true;
                 }

               #if !defined(FERRET_DISABLE_STD_OUT)
                 void stream_console() const final {
                   runtime::print("()");
                 }
               #endif
               };

               namespace cached{
                 const var empty_sequence = obj<ferret::empty_sequence>();
               }

               namespace runtime {
                 struct range{
                   var p;

                   explicit range(var const & v) : p(v) { }
                   inline range begin() const { return range(p); }
                   inline range end()   const { return range(cached::empty_sequence); }

                   inline bool operator!=(const range& other){
                     return !p.is_nil() && (p != other.p);
                   }

                   inline const range& operator++(){
                     p = runtime::rest(p);
                     return *this;
                   }

                   inline var operator*(){
                     return runtime::first(p);
                   }
                 };
               }

               namespace runtime {
                 struct range_indexed_pair{
                   number_t index;
                   var value;

                   explicit range_indexed_pair(number_t i = 0, var const & v = nil()) : index(i) , value(v) { }
                 };
                 
                 struct range_indexed{
                   var p;
                   number_t index;

                   explicit range_indexed(var const & v) : p(v) , index(0) { }
                   inline range_indexed begin() const { return range_indexed(p); }
                   inline range_indexed end()   const { return range_indexed(cached::empty_sequence); }

                   inline bool operator!=(const range_indexed& other){
                     return !p.is_nil() && (p != other.p);
                   }

                   inline const range_indexed& operator++(){
                     p = runtime::rest(p);
                     index++;
                     return *this;
                   }

                   inline range_indexed_pair operator*(){
                     return range_indexed_pair(index, runtime::first(p));
                   }
                 };
               }

               namespace runtime {
                 struct range_pair_pair{
                   var first;
                   var second;

                   explicit range_pair_pair(var const & a = nil(), var const & b = nil()) : first(a) , second(b) { }
                 };
                   
                 struct range_pair{
                   var first;
                   var second;

                   explicit range_pair(var const & a = nil(), var const & b = nil()) : first(a) , second(b) { }
                   
                   inline range_pair begin() const { return range_pair(first, second); }
                   inline range_pair end()   const { return range_pair(cached::empty_sequence,cached::empty_sequence); }

                   inline bool operator!=(const range_pair& other){
                     return (first != other.first) && (second != other.second);
                   }

                   inline const range_pair& operator++(){
                     first = runtime::rest(first);
                     second = runtime::rest(second);
                     return *this;
                   }

                   inline range_pair_pair operator*(){
                     return range_pair_pair(runtime::first(first), runtime::first(second));
                   }
                 };
               }
               #endif
         #ifndef FERRET_OBJECT_SEQUENCE
               #define FERRET_OBJECT_SEQUENCE
                 namespace runtime {
                   namespace type {
                      const size_t sequence = 3956;}}

               class sequence final : public object, public seekable_i {
                 const var next;
                 const var data;
               public:

                 size_t type() const final { return runtime::type::sequence; }

                 bool equals(var const & o) const final {
                   if(first() != runtime::first(o))
                     return false;
                   
                   for(auto const& it : runtime::range_pair(rest(),runtime::rest(o)))
                     if (it.first != it.second)
                       return false;

                   return true;
                 }

               #if !defined(FERRET_DISABLE_STD_OUT)
                 void stream_console() const final {
                   runtime::print("(");
                   data.stream_console();
                   for(auto const& i : runtime::range(next)){
                     runtime::print(" ");
                     i.stream_console();
                   }
                   runtime::print(")");
                 }
               #endif

                 explicit sequence(var const & d = nil(), var const & n = nil()) : next(n), data(d) {} 

                 virtual seekable_i* cast_seekable_i() { return this; }

                 var cons(var const & x) final {
                   return obj<sequence>(x, var(this));
                 }
                 var first() const final {
                   return data;
                 }
                 var rest() const final {
                   return next;
                 }
                 template <typename T>
                 static T to(var const & ){
                   T::unimplemented_function;
                 }
                 template <typename T>
                 static var from(T){
                   T::unimplemented_function; return nil();
                 }

               };
               namespace runtime {
                 inline var list() { 
                   return cached::empty_sequence;
                 }
                 inline var list(var const & v) { 
                   return obj<sequence>(v,nil());
                 }
                                   
                 template <typename... Args>
                 inline var list(var const & first, Args const & ... args) { 
                   return obj<sequence>(first, list(args...));
                 }
               }

               #ifdef FERRET_STD_LIB
               typedef ::std::vector<var>  std_vector;

               template <> std_vector sequence::to(var const & v) { 
                 std_vector ret;
                 for(auto const& it : runtime::range(v))
                   ret.push_back(it);
                 return ret;
               }

               template <> var sequence::from(std_vector v) { 
                 var ret;
                 for(auto const& it : v)
                   ret = runtime::cons(it,ret);
                 return ret;
               }
               #endif
               #endif
         #ifndef FERRET_OBJECT_LAZY_SEQUENCE
               #define FERRET_OBJECT_LAZY_SEQUENCE
                 namespace runtime {
                   namespace type {
                      const size_t lazy_sequence = 3957;}}

               class lazy_sequence final : public object, public seekable_i {
                 const var thunk;
                 const var head;
               public:

                 size_t type() const final { return runtime::type::lazy_sequence; }

                 var sval() const {
                   if (head.is_nil())
                     return runtime::first(run(thunk));
                   
                   return head;
                 }

                 bool equals(var const & o) const final {
                   if(sval() != runtime::first(o))
                     return false;
                   
                   for(auto const& it : runtime::range_pair(rest(),runtime::rest(o)))
                     if (it.first != it.second)
                       return false;

                   return true;
                 }

               #if !defined(FERRET_DISABLE_STD_OUT)
                 void stream_console() const final {
                   runtime::print("(");
                   sval().stream_console();
                   for(auto const& i : runtime::range(rest())){
                     runtime::print(" ");
                     i.stream_console();
                   }
                   runtime::print(")");
                 }
               #endif

                 explicit lazy_sequence(var const & t) : thunk(t) {} 
                 explicit lazy_sequence(var const & h, var const & t) : thunk(t), head(h) {} 

                 virtual seekable_i* cast_seekable_i() { return this; }

                 var cons(var const & x) final {
                   if (!head.is_nil())
                     return obj<sequence>(x, obj<lazy_sequence>(head,thunk));

                   return obj<lazy_sequence>(x,thunk);
                 }
                 var first() const final {
                   return sval();
                 }
                 var rest() const final {
                   if (head.is_nil())
                     return runtime::rest(run(thunk));
                   
                   return run(thunk);
                 }
               };
               #endif
         #ifndef FERRET_OBJECT_D_LIST
               #define FERRET_OBJECT_D_LIST
                 namespace runtime {
                   namespace type {
                      const size_t d_list = 3958;}}

               class d_list final : public lambda_i, public seekable_i {

                 var data;

                 number_t val_index(var const & k) const {
                   var keys = runtime::first(data);

                   for(auto i : runtime::range_indexed(keys))
                     if ( i.value == k )
                       return i.index;

                   return -1;
                 }
                 
               public:

                 size_t type() const final { return runtime::type::d_list; }

                 bool equals(var const & o) const final {
                   return (this == o.get());
                 }

               #if !defined(FERRET_DISABLE_STD_OUT)
                 void stream_console() const final {
                   data.stream_console();
                 }
               #endif

                 explicit d_list() : data(runtime::list(runtime::list())) { }
                 explicit d_list(var const & l) : data(l) { }

                 var assoc(var const & k, var const & v) const {
                   var keys = runtime::first(data);
                   var values = runtime::rest(data);

                   values = runtime::cons(v,values);
                   keys   = runtime::cons(k,keys);
                   
                   return obj<d_list>(runtime::cons(keys,values));
                 }

                 var dissoc(var const & k) const {
                   number_t idx = val_index(k);
                   
                   if ( idx == -1 )
                     return obj<d_list>(data);

                   var keys = runtime::first(data);
                   var values = runtime::rest(data);

                   var new_keys;
                   for(auto i : runtime::range_indexed(keys))
                     if ( i.index != idx)
                       new_keys = runtime::cons(i.value, new_keys);

                   var new_values;
                   for(auto i : runtime::range_indexed(values))
                     if ( i.index != idx)
                       new_values = runtime::cons(i.value, new_values);
                   
                   return obj<d_list>(runtime::cons(new_keys,new_values));
                 }
                 
                 var val_at(var const & args) const {
                   var key = runtime::first(args);
                   var not_found = runtime::first(runtime::rest(args));

                   var values = runtime::rest(data);
                   number_t idx = val_index(key);

                   if ( idx == -1 ){
                    if ( !not_found.is_nil() ){
                     return not_found;
                    }else{
                     return nil();  
                    }
                   }

                   for(number_t i = 0; i < idx; i++)
                     values = runtime::rest(values);
                   
                   return runtime::first(values);
                 }

                 var invoke(var const & args) const final {
                   return val_at(args);
                 }

                 var vals () const { return runtime::rest(data);}
                 var keys () const { return runtime::first(data);}

                 virtual seekable_i* cast_seekable_i() { return this; }
                 
                 var cons(var const & v) final {
                   return runtime::list(v,data);
                 }
                 
                 var first() const final {
                   var keys = runtime::first(data);
                   var values = runtime::rest(data);
                   return runtime::list(runtime::first(keys),runtime::first(values));
                 }
                 
                 var rest() const final {
                   var keys = runtime::first(data);
                   var values = runtime::rest(data);

                   if(runtime::rest(keys) == nil())
                     return runtime::list();
                   
                   return obj<d_list>(runtime::cons(runtime::rest(keys),runtime::rest(values)));
                 }
               };
               #endif
         #ifndef FERRET_OBJECT_KEYWORD
               #define FERRET_OBJECT_KEYWORD
                 namespace runtime {
                   namespace type {
                      const size_t keyword = 3959;}}

               class keyword final : public lambda_i {
                 const number_t _word;

                 number_t from_str(const char * str){
                   number_t word = 0;
                   for (number_t i = 0; str[i] != '\0'; i++){
                     word = word + (number_t)str[i];
                   }
                   
                   return word;
                 }
                 
               public:

                 size_t type() const final { return runtime::type::keyword; }

                 bool equals(var const & o) const final {
                   return (_word == o.cast<keyword>()->word());
                 }

               #if !defined(FERRET_DISABLE_STD_OUT)
                 void stream_console() const final {
                   runtime::print("keyword<");
                   runtime::print(_word);
                   runtime::print(">");
                 }
               #endif

                 explicit keyword(number_t w) : _word(w) {} 
                 explicit keyword(const char * str): _word(from_str(str)) { }

                 number_t word() const {
                   return _word;
                 }

                 var invoke(var const & args) const {
                   var map = runtime::first(args);
                   var map_args = runtime::cons(var((object*)this), runtime::rest(args));

                   if (map.is_type(runtime::type::d_list)){
                     return map.cast<d_list>()->val_at(map_args);
                   }

                   return nil();
                 }
               };
               #endif
         #ifndef FERRET_OBJECT_STRING
               #define FERRET_OBJECT_STRING
                 namespace runtime {
                   namespace type {
                      const size_t string = 3960;}}

               class string final : public object, public seekable_i {
                 var data;

                 void from_char_pointer(const char * str, int length){
                   for (int i = --length; i >= 0; i--)
                     data = runtime::cons(obj<number>((number_t)str[i]),data);
                 }
                 
               public:

                 size_t type() const final { return runtime::type::string; }

                 bool equals(var const & other) const final {
                   return (container() == other.cast<string>()->container());
                 }

               #if !defined(FERRET_DISABLE_STD_OUT)
                 void stream_console() const final {
                   for(auto const& it : runtime::range(data))
                     runtime::print(number::to<char>(it));
                 }
               #endif

                 explicit string() : data(nullptr) {} 

                 explicit string(var const & s) : data(s) {}

                 explicit string(const char * str) {
                   int length = 0;
                   for (length = 0; str[length] != '\0'; ++length);
                   from_char_pointer(str,length);
                 }

                 explicit string(const char * str,number_t length) { from_char_pointer(str,length); }

                 var container() const {
                   return data;
                 }

                 virtual seekable_i* cast_seekable_i() { return this; }

                 var cons(var const & x) final {
                   return obj<string>(runtime::cons(x,data));
                 }

                 var first() const final {
                   return runtime::first(data);
                 }

                 var rest() const final {
                   if (!runtime::rest(data).is_nil())
                     return obj<string>(runtime::rest(data));

                   return cached::empty_sequence;
                 }

                 template <typename T>
                 static T to(var const & ){
                   T::unimplemented_function;
                 }

               };

               #ifdef FERRET_STD_LIB
               template<>
               inline var obj<string>(std::string s) {
                 void * storage = FERRET_ALLOCATOR::allocate<string>();
                 return var(new(storage) string(s.c_str(), (number_t)s.size()));
               }

               template <> ::std::string string::to(var const & v) { 
                 ::std::stringstream ss;
                 for(auto const& it : runtime::range(v.cast<string>()->container()))
                   ss << number::to<char>(it);
                 return ss.str();
               }
               #endif
               #endif
         #ifndef FERRET_OBJECT_ATOMIC
               #define FERRET_OBJECT_ATOMIC
                 namespace runtime {
                   namespace type {
                      const size_t atomic = 3961;}}

               class atomic final : public deref_i {
                 var data;
                 mutex lock;
                 public:


                 size_t type() const final { return runtime::type::atomic; }

                 bool equals(var const & o) const final {
                   return (this == o.cast<atomic>());
                 }

               #if !defined(FERRET_DISABLE_STD_OUT)
                 void stream_console() const final {
                   runtime::print("atom<");
                   data.stream_console();
                   runtime::print(">");
                 }
               #endif

                 explicit atomic(var const & d) : data(d) {} 

                 var swap(var const & f,var const & args){
                   lock_guard guard(lock);
                   data = f.cast<lambda_i>()->invoke(runtime::cons(data, args));
                   return data;
                 }
                 var deref() {
                   lock_guard guard(lock);
                   return data;
                 }
               };
               #endif
         #ifndef FERRET_OBJECT_ASYNC
               #define FERRET_OBJECT_ASYNC
                 namespace runtime {
                   namespace type {
                      const size_t async = 3962;}}

               #ifdef FERRET_STD_LIB
               class async final : public deref_i {
                 var value;
                 mutex lock;
                 var fn;
                 bool cached;
                 std::future<var> task;

                 class rc_guard{
                   object *obj;
                 public:
                   explicit rc_guard(const rc_guard &) = delete;
                   explicit rc_guard(object *o) : obj(o) { };
                   ~rc_guard() { obj->dec_ref(); }
                 };

                 var exec() {
                   rc_guard g(this);
                   return run(fn);
                 }
                 
                 public:

                 explicit async(var const & f) :
                   value(nil()), fn(f), cached(false),
                   task(std::async(std::launch::async, [this](){ return exec(); })){ inc_ref(); }

                 size_t type() const final { return runtime::type::async; }

                 bool equals(var const & o) const final {
                   return (this == o.cast<async>());
                 }

               #if !defined(FERRET_DISABLE_STD_OUT)
                 void stream_console() const final {
                   runtime::print("future<");
                   fn.stream_console();
                   runtime::print(">");
                 }
               #endif

                 bool is_ready(){
                   lock_guard guard(lock);
                   if (cached)
                     return true;
                   return task.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
                 }

                 void get(){
                   if (!cached){
                     value = task.get();
                     cached = true;
                   }
                 }

                 var deref() {
                   lock_guard guard(lock);
                   get();
                   return value;
                 }
               };
               #endif
               #endif
         #ifndef FERRET_OBJECT_DELAYED
               #define FERRET_OBJECT_DELAYED
                 namespace runtime {
                   namespace type {
                      const size_t delayed = 3963;}}

               class delayed final : public deref_i {
                 var val;
                 mutex lock;
                 var fn;
                 
                 public:

                 size_t type() const final { return runtime::type::delayed; }

                 bool equals(var const & o) const final {
                   return (this == o.cast<delayed>());
                 }

               #if !defined(FERRET_DISABLE_STD_OUT)
                 void stream_console() const final {
                   runtime::print("delay");
                 }
               #endif

                 explicit delayed(var const & f) : fn(f) {} 
                 
                 var deref() {
                   lock_guard guard(lock);
                   if (!fn.is_nil()){
                     val = fn.cast<lambda_i>()->invoke(nil());
                     fn = nil();
                   }
                   return val;
                 }
               };
               #endif
         #ifndef FERRET_OBJECT_ELAPSED_MICROS
               #define FERRET_OBJECT_ELAPSED_MICROS
                 namespace runtime {
                   namespace type {
                      const size_t elapsed_micros = 3964;}}

               #if !defined(FERRET_SAFE_MODE)
               class elapsed_micros : public object {

                 unsigned long us;

               #if defined(FERRET_HARDWARE_ARDUINO)
                 inline unsigned long now() const{
                   return ::micros();
                 }
               #elif defined(FERRET_STD_LIB)
                 inline unsigned long now() const{
                   auto now = ::std::chrono::high_resolution_clock::now();
                   auto epoch = now.time_since_epoch();
                   return (unsigned long)::std::chrono::duration_cast<::std::chrono::microseconds>(epoch).count();
                 }
               #endif

                 inline unsigned long _elapsed() const { return (now() - us); }  
                 
                public:

                 elapsed_micros(void) { us = now(); }
                 void reset() { us = now(); }
                 
                 size_t type() const { return runtime::type::elapsed_micros; }

                 bool equals(var const & o) const {
                   return (this == o.cast<elapsed_micros>());
                 }

               #if !defined(FERRET_DISABLE_STD_OUT)
                 void stream_console() const {
                   runtime::print("elapsed_micros<");
                   runtime::print(_elapsed());
                   runtime::print(">");
                 }
               #endif

                 inline var elapsed() const { return obj<number>(_elapsed()); }
                 inline bool is_elapsed(real_t t) const { return (_elapsed() >= (unsigned long)t); }
               };
               #endif
               #endif
         #ifndef FERRET_OBJECT_PID_CONTROLLER
               #define FERRET_OBJECT_PID_CONTROLLER
                 namespace runtime {
                   namespace type {
                      const size_t pid_controller = 3965;}}

               template <typename T>
               class pid_controller : public lambda_i {
                 T p;
                 T i;
                 T d;
                 T maximum_output;
                 T minimum_output;
                 T maximum_input;
                 T minimum_input;
                 bool continuous;
                 var setpoint_fn;
                 mutable T setpoint;
                 mutable T prev_error;
                 mutable T total_error;
                 mutable T error;
                 mutable T result;
                 mutable T input;
               public:

                 pid_controller(var const & kp, var const & ki, var const & kd,
                                var const & inMin, var const & inMax, var const & outMin, var const & outMax,
                                var const & cont,
                                var const & sp){
                   p = number::to<T>(kp);
                   i = number::to<T>(ki);
                   d = number::to<T>(kd);
                   maximum_output = number::to<T>(outMax);
                   minimum_output = number::to<T>(outMin);
                   maximum_input = number::to<T>(inMax);
                   minimum_input = number::to<T>(inMin);
                   continuous = cont.cast<boolean>()->container();

                   if (sp.is_type(runtime::type::lambda_i)){
                     setpoint_fn = sp;
                     set_setpoint(run(setpoint_fn));
                   }else{
                     set_setpoint(sp);
                   }

                   prev_error = 0;
                   total_error = 0;
                   error = 0;
                   result = 0;
                   input = 0;
                 }

                 size_t type() const { return runtime::type::pid_controller; }

                 bool equals(var const & o) const {
                   return (this == o.cast<pid_controller>());
                 }

               #if !defined(FERRET_DISABLE_STD_OUT)
                 void stream_console() const {
                   runtime::print("pid_controller");
                 }
               #endif

                 var update(var const & in) const {
                   input = number::to<T>(in);

                   // Calculate the error signal
                   error = setpoint - input;

                   // If continuous is set to true allow wrap around
                   if (continuous) {
                     if (runtime::abs(error) > ((maximum_input - minimum_input) / (real_t)2)) {
                       if (error > (real_t)0) {
                         error = (error - maximum_input) + minimum_input;
                       } else {
                         error = (error + maximum_input) - minimum_input;
                       }
                     }
                   }
                                             
                   /*
                    * Integrate the errors as long as the upcoming integrator does
                    * not exceed the minimum and maximum output thresholds
                    */
                   if ((((total_error + error) * i) < maximum_output) &&
                       (((total_error + error) * i) > minimum_output)) {
                     total_error += error;
                   }
                                             
                   // Perform the primary PID calculation
                   result = ((p * error) + (i * total_error) + (d * (error - prev_error)));
                                             
                   // Set the current error to the previous error for the next cycle
                   prev_error = error;
                                             
                   // Make sure the final result is within bounds
                   if (result > maximum_output) {
                     result = maximum_output;
                   } else if (result < minimum_output) {
                     result = minimum_output;
                   }

                   return obj<number>(result);
                 }
                 
                 void set_setpoint(var const & p) const {
                   T sp = number::to<T>(p);
                   if (maximum_input > minimum_input) {
                     if (sp > maximum_input) {
                       setpoint = maximum_input;
                     } else if (sp < minimum_input) {
                       setpoint = minimum_input;
                     } else {
                       setpoint = sp;
                     }
                   } else {
                     setpoint = sp;
                   }
                 }

                 var invoke(var const & args) const {
                   if (!setpoint_fn.is_nil())
                     set_setpoint(run(setpoint_fn));
                   return update(runtime::first(args));
                 }
                 
                 void reset(){
                   prev_error = 0;
                   total_error = 0;
                   result = 0;
                 }
               };
               #endif
         #ifndef FERRET_OBJECT_MOVING_AVERAGE_FILTER
               #define FERRET_OBJECT_MOVING_AVERAGE_FILTER
                 namespace runtime {
                   namespace type {
                      const size_t moving_average_filter = 3966;}}

               template <typename T>
               class moving_average_filter : public lambda_i {
                 T alpha;
                 mutable T avrg;
               public:

                 explicit moving_average_filter(var const & a) : avrg(0) {
                   alpha = number::to<T>(a);
                 }

                 size_t type() const { return runtime::type::moving_average_filter; }

                 bool equals(var const & o) const {
                   return (this == o.cast<moving_average_filter>());
                 }

               #if !defined(FERRET_DISABLE_STD_OUT)
                 void stream_console() const {
                   runtime::print("moving_average_filter<");
                   runtime::print(alpha);
                   runtime::print(",");
                   runtime::print(avrg);
                   runtime::print(">");
                 }
               #endif

                 var invoke(var const & args) const {
                   T data = number::to<T>(runtime::first(args));
                   avrg = ((alpha * data) + ((1. - alpha) * avrg));
                   return obj<number>(avrg);
                 }
               };
               #endif
        }

        namespace f { 
         using namespace ferret;
         typedef ferret::boolean boolean; 
        }

        // Symbols
        namespace f{
        }


        // Runtime Implementations
        #ifndef FERRET_RUNTIME_CPP
        #define FERRET_RUNTIME_CPP

         namespace ferret{
           namespace runtime{
             var first(var const & coll){
               if (coll.is_nil() || coll.is_type(runtime::type::empty_sequence))
                 return nil();
               else
                 return coll.cast<seekable_i>()->first();
             }
           
             var rest(var const & coll){
               if (coll.is_nil())
                 return runtime::list();
               if (coll.is_type(runtime::type::empty_sequence))
                 return nil();
               return coll.cast<seekable_i>()->rest();
             }
           
             var cons(var const & x, var const & coll){
               if (coll.is_nil() || coll == runtime::list())
                 return runtime::list(x);
           
               return coll.cast<seekable_i>()->cons(x);
             }
           
             var nth(var const & seq, number_t index){
               for(auto const& i : range_indexed(seq))
                 if (index == i.index)
                   return i.value;
           
               return nil();
             }
           
             var nthrest(var const & seq, number_t index){
               var ret = seq;
               for(number_t i = 0; i < index; i++)
                 ret = runtime::rest(ret);
           
               if (ret.is_nil())
                 return runtime::list(); 
           
               return ret;
             }
             
             size_t count(var const & seq){
               size_t acc = 0;
               for(auto const& v : runtime::range(seq)){
                 (void)v;
                 acc++;
               }
               return acc;
             }
           
             bool is_seqable(var const & seq){
               if(seq.cast<seekable_i>())
                 return true;
               else
                 return false;
             }
           }
           template<typename T, typename... Args>
           inline var run(T const & fn, Args const & ... args) {
             return fn.invoke(runtime::list(args...));
           }
           
           template<typename T>
           inline var run(T const & fn) {
             return fn.invoke(nil());
           }
           
           template<>
           inline var run(var const & fn) {
             return fn.cast<lambda_i>()->invoke(nil());
           }
           
           template<typename... Args>
           inline var run(var const & fn, Args const & ... args) {
             return fn.cast<lambda_i>()->invoke(runtime::list(args...));
           }
         }
        #endif

        // Lambda Prototypes
        namespace f{
                 class second  {
                public:

                  var invoke (var const & _args_) const  ;
                };

                 class apply  {
                public:

                  var invoke (var const & _args_) const  ;
                };

                 class G__3944  {
                  const var more__3685;
                public:
                    explicit G__3944 (var const & more__3685) :
                      more__3685(more__3685) { }

                  var invoke (var const & _args_) const  ;
                };

                 class println  {
                public:

                  var invoke (var const & _args_) const  ;
                };

                 class print final : public lambda_i{
                public:

                  var invoke (var const & _args_) const  final  ;
                };

                 class newline  {
                public:

                  var invoke (var const & _args_) const  ;
                };
        }

        // Command Line Arguments
        #if defined(FERRET_STD_LIB) &&               \
            !defined(FERRET_DISABLE_CLI_ARGS) &&     \
            !defined(FERRET_DISABLE_STD_MAIN)
          ferret::var _star_command_line_args_star_;
        #endif

        // Lambda Implementations
        namespace f{
                inline var second::invoke (var const & _args_) const {
                  (void)(_args_);
                  const var x = runtime::first(_args_);
             
                  var __result;
                  __result = runtime::first(runtime::rest(x));;
                  return __result;
                }
               

                inline var apply::invoke (var const & _args_) const {
                  (void)(_args_);
                  const var f = runtime::first(_args_);
                  const var args = runtime::first(runtime::rest(_args_));
             
                  var __result;
                  __result = f.cast<lambda_i>()->invoke(args);;
                  return __result;
                }
               

                inline var G__3944::invoke (var const & _args_) const {
                  (void)(_args_);
             
                  return run(apply(),obj<print>(),more__3685);
                }
               

                inline var println::invoke (var const & _args_) const {
                  (void)(_args_);
                  const var more__3685 = _args_;
             
                  (more__3685 ? run(G__3944(more__3685)) : nil());
                  return run(newline());
                }
               

                inline var print::invoke (var const & _args_) const {
                  (void)(_args_);
                  const var more = _args_;
             

                  #if !defined(FERRET_DISABLE_STD_OUT) 
                  if (more.is_nil())
                           return nil();
                         var f = runtime::first(more);
                         f.stream_console();
                         var r = runtime::rest(more);
                         for(auto const& it : runtime::range(r)){
                          runtime::print(" ");
                          it.stream_console();
                         }
                  #endif
                  ;
                  return nil();
                }
               

                inline var newline::invoke (var const & _args_) const {
                  (void)(_args_);
             

                  #if !defined(FERRET_DISABLE_STD_OUT) 
                  runtime::print("\n");
                  #endif
                  ;
                  return nil();
                }
               
        }

        // Program Run
        namespace f{
         void main(){
          run(println(),run(second(),_star_command_line_args_star_)); 
         }
        }


        #if !defined(FERRET_DISABLE_STD_MAIN)
         #if defined(FERRET_DISABLE_CLI_ARGS) || !defined(FERRET_STD_LIB)
          int main()
         #else
          int main(int argc, char* argv[])
         #endif
          {     
            using namespace ferret;
            FERRET_ALLOCATOR::init();
            runtime::init();

           #if defined(FERRET_STD_LIB) && !defined(FERRET_DISABLE_CLI_ARGS)
            for (int i = argc - 1; i > -1 ; i--)
              _star_command_line_args_star_ =  runtime::cons(obj<string>(argv[i]),_star_command_line_args_star_);
           #endif

            f::main();

           #if defined(FERRET_PROGRAM_MAIN)
            run(FERRET_PROGRAM_MAIN);
           #endif
             
            return 0;
          }
        #endif
        #if defined(FERRET_HARDWARE_ARDUINO)
          void setup(){
            using namespace ferret;
            FERRET_ALLOCATOR::init();
            runtime::init();

            #if defined(FERRET_PROGRAM_MAIN)
              f::main();
            #endif
          }

          void loop(){
            using namespace ferret;
            #if !defined(FERRET_PROGRAM_MAIN)
              f::main();
            #endif          

            #if defined(FERRET_PROGRAM_MAIN)
              run(FERRET_PROGRAM_MAIN);
            #endif
          }
        #endif