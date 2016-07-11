Parsing
=======

Parsing is an extensive subject and a frequent stumbling block of even experienced engineers.  Due to the repetitive nature of parsing code, the volume of the points of failure and the frequently changing format of inputs, parsing code is often a significant resource sink for many software projects.

Parsing is frequently outlined with a grammar specification in [Backus-Naur Form (BNF)](https://en.wikipedia.org/wiki/Backus%E2%80%93Naur_Form) which defines the terminals and non-terminals that must be tagged in the input stream. Some tools accept the BNF grammar directly as input and generate all the parsing code as output. These tools are called parser-generators and they can save a lot of pain and time. Though not limited to accepting BNF exclusively as inputs, some parser-generates have custom language specifications or decorate BNF with various text handling code, the basic product is normally the same: some language specification is taken as input and all the boiler plate parsing code is generated as output. In most cases the resulting generated parser includes some interfaces for the library consumer to interrogate the parsed sources. Frequently, the parsed inputs are then feed as inputs to another stage which produces an [Abstract Syntax Tree (AST)](https://en.wikipedia.org/wiki/Backus%E2%80%93Naur_Form). The AST is most often the primary model that applications use to work with the inputs.

Some applications do not use generated parsers nor an AST. Some apps do not even abstract the parsing code from the processing code and will instead parse the inputs directly in-line with application logic. For all except the most trivial applications this is a recipe for disaster. Abstracting the two tasks is almost always the preferred method. However, doing things 'right' and adhering to best practice is also time consuming. For example, using the traditional GNU tools to handle parsing is an entire discipline of it's own that complicates the build process and requires a modicum of domain expertise before any application logic is ever addressed. Modern tools such as Boost::Spirit and Antlr suffer from the same problem of requiring a detailed study of the library to get a simple parse task accomplished.

When the introduction of a tool that is intended to simplify a problem makes the problem more complicated the tool looses it's utility and usefullness. Since using these tools require a significant investiment to learn and use properly it's no surprise that so many developers just opt to hand-roll a parser rather than learn yet another library. With all that said I introduce yet another library.

Introduction
------------

XTL::parse uses template meta-programming techniques to generate parse trees from a grammar specification.  The grammar specification is written in C++ templates. The library is header-only and the parse trees are generated at compile time so there's no libraries to link to and no external tools to run as part of the build process. A unique feature of XTL::parse is that the grammar specification gets instantiated as the [AST] when the parse is successful which eleminates the need for an additional import step which is frequent of other similar tools. XTL::parse is a simple LL(k) parser that encourages embedding the grammar specification in the AST for simplicity and clarity.

To illustrate the entire process, here's a simple BNF grammar describing the command line syntax of example_parse1.cpp:

~~~{.cpp}
//terminals
<red>           := 'red'
<green>         := 'green'
<blue>          := 'blue'
<one>           := '1'
<three>         := '3'
<five>          := '5'
<dash_color>    := '--color='
<dash_prime>    := '--prime='
//rules
<rgb>           := <red> | <green> | <blue>
<prime_num>     := <one> | <three> | <five>
<color_param>   := <dash_color> <rgb>
<prime_param>   := <dash_prime> <prime_num>
<parameter>     := <color_param> | <prime_param>
~~~
> If the intention of this BNF syntax is unclear there are plenty of tutorials on the web.

This is the sort of BNF that is frequently encountered in RFCs, white papers, programming language and protocol specifications. It maps into a XTL::parse grammar specification as:

~~~{.cpp}
//termnals
STRING(red, "red");
STRING(green, "green");
STRING(blue, "blue");
STRING(one, "1");
STRING(three, "3");
STRING(five, "5");
STRING(dash_color, "--color=");
STRING(dash_prime, "--prime=");
//rules
using rgb = or_<red, green, blue>;
using prime_num = or_<one, three, five>;
using color_param = and_<dash_color, rgb>;
using prime_param = and_<dash_prime, prime_num>;
using parameter = or_<color_param, prime_param>;
~~~

The `parameter` maybe either `--prime=<prime_num>` or `--color=<rgb>`. `<prime_num>` and `<rgb>` maybe either `red, green` or `blue` and `1, 3` or  `5` respectively. This example uses a mixture of preprocessor macros and template aliases to define the grammar. The C++ representation is more verbose due to C++ language requirements but it maps to the BNF line-for-line.

Using this specification to parse command line parameters is a matter of passing the start rule and string to a parser:

~~~{.cpp}
int main(int argc, char * argv[]){
    std::string sParam = argv[1];
    auto oAST = xtd::parser<parameter>::parse(sParam.begin(), sParam.end());
    if (!oAST){
        //parse failed, show usage or error
    }else{
        //work with parsed parameters
    }
}
~~~
Done and done.

AST Generation
--------------
An AST is instantated and returned by `xtd::parser<>::parse()` when the parse is successful. The AST is an object model that represents the parsed grammar.
