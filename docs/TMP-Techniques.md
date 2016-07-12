TMP Techniques
==============
There are a few patterns in wide-spread employ across the TMP landscape and it's worth recognizing them. Here are a few of the more common patterns/idioms with examples.

### Use before declaration
Template declarations are parsed at compile time and not instantiated until their use is defined in run-time code. If a template is not used in run-time code then it can contain a myriad of errors and the application will compile just fine since it's not used. This permits template to contain code that operates on template parameters before the concrete types are declared effectively decoupling two class declarations in ways that aren't possible in classic C++. For example, the following will not compile:
```{.cpp}
struct Cat{
   Mouse m_Mouse;
};
struct Mouse{
  int m_Cheese;
};

int main(){
  return 0;
}
```
Neither Cat nor Mouse are used in run-time code but it won't compile because Cat reference Mouse before it's declared and forward declarations won't help. However, consider the following:
```{.cpp}
template <typename _MemberT> struct Cat{
  _MemberT m_Mouse;
};
struct Mouse{
  int m_Cheese;
};
int main(){
  Cat<Mouse> oCat;
}
```
The templated version compiles just fine because the template isn't fully resolved until it's used in the run-time code in `main` (`Cat<Mouse> oCat`.) At this point the template becomes instantiated. All the types are fully defined at the time they're used in run-time code so everything works.

Expanding on this idiom, a template can interrogate the parameter:
```{.cpp}
template <typename _MouseT> struct Cat{
  static bool Catch(){
    return _MouseT::Slow;
  }
};
struct MickeyMouse{
  static const bool Slow = true;
};
struct MightyMouse{
  static const bool Slow = false;
}
int main(){
  std::cout << "Catch Mickey " << std::boolalpha << Cat<MickeyMouse>::Catch() << std::endl;
  std::cout << "Catch Mighty " << std::boolalpha << Cat<MightyMouse>::Catch() << std::endl;
}
```
Here again `Cat` uses `_MouseT` but also uses a member named `Slow`. Using members of a template parameter like this requires that any type passed to the `Cat` template have a `Slow` member. In other words, the `_MouseT` parameter must adhere to a _concept_ that `Cat` requires. As long as the type passed as `_MouseT` to the `Cat` template adheres to the _interface concept_ the code is valid.

Performing similar feats in classic C++ normally requires a generic interface such as `IMouse` be declared with some pure virtual members that subclasses implement. 

### Curiously Recurring Template Pattern
This one looks odd at first glance and seems that it shouldn't compile. Here's a common use care with STL:
```{.cpp}
struct MyStruct : std::enable_shared_from_this<MyStruct> ...
```
The MyStruct passes itself as a parameter to specialize std::enable_shared_from_this before the definition is complete. It doesn't look like it will compile but it does. This pattern is often used to give the base class visibility into the subclasses, something that cannot be easily done in classic OO.

### Parameterized Base Class
This one also appears curious at first:
```{.cpp}
template <typename _BaseT> struct MyStruct : _BaseT ...
```
MyStruct is a template that subclasses it's template parameter. 
