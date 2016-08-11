Hierarchy Generation
====================
This is a TMP pattern to generate a class hierarchy from a collection of policy classes. It permits arbitrarily mixing policies to generate rich components while avoiding multiple inheritance. It works by chaining together a series of policies in a linear class hierarchy through a couple controller templates.

The various policies are required to implement a parameterized super class idiom:
~~~{.cpp}
template <class _SuperT> class OnClick : _SuperT {
    ...
};
template <class _SuperT> class OnPaint : _SuperT {
    ...
};
template <class _SuperT> class OnResize : _SuperT {
    ...
};
template <class _SuperT> class OnTimer : _SuperT {
    ...
};
~~~
Each policy template is a self contained behavioral unit that describes an aspect of the larger whole. To maximize reuse they should be independent but it's possible to share information between them if care is taken.
 
The process of chaining the policies together is performed through a controller with the following declaration:

~~~{.cpp}
template <class _BaseT, template <class> class ... _PolicyList> 
class HierarchyGenerator;
~~~

The parameters of the `HierarchyGenerator` are `_BaseT` and `_PolicyList`. The `_BaseT` parameter in this example is the super class of the entire hierarchy and will normally be a non-templated class. Policies are passed as parameters to the `HierarchyGenerator` in the `_PolicyList` parameter to successively declare a linear class hierarchy:

~~~{.cpp}
class Window{
...
};
using Button = HierarchyGenerator<Window, OnClick, OnPaint, OnResize>;
using Label = HierarchyGenerator<Window, OnPaint>;
using Sizer = HierarchyGenerator<Window, OnClick, OnPaint, OnResize, OnTimer>;
using Timer = HierarchyGenerator<Window, OnTimer>;
~~~
This results in linear class hierarchies composed of a mixture of policies. It equivalently declares the following:

~~~{.cpp}
using Button = OnClick<OnPaint<OnResize<Window>>>;
using Label = OnPaint<Window>;
using Sizer = OnClick<OnPaint<OnResize<OnTimer<Window>>>>;
using Timer = OnTimer<Window>;
~~~
The two forms are the same and the varation using the `HierarchyGenerator` is more verbose but the intent is clearer and more readable.

The 'magic' is done through a couple partial specializations of `HierarchyGenerator`:
~~~{.cpp}
template <typename _BaseT> class HierarchyGenerator<_BaseT> : public _BaseT{
    template <typename ... _Args> HierarchyGenerator(_Args&&...oArgs) : _BaseT(std::forward<_Args>(oArgs)...){}
};
~~~
This is the final specialization in the recusive template instantation chain that makes `_BaseT` the base class of all the policies. The compiler selects this specialization when the `_PolicyList` has been depleted of items. Chaining together the items in the `_PolicyList` is performed with the following partial specialization:
~~~
template <typename _BaseT, template <class> class _PolicyT, template <class> class ... _PolicyList>
class HierarchyGenerator<_BaseT, _PolicyT, _PolicyList...> : public _PolicyT< HierarchyGenerator<_PolicyList...>> {
    template <typename ... _Args> HierarchyGenerator(_Args&&...oArgs) 
        : _PolicyT< HierarchyGenerator<_PolicyList...>>(std::forward<_Args>(oArgs)...){}
};
~~~
