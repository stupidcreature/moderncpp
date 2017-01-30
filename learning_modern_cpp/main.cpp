#include <algorithm>
#include <array>
#include <ctime>
#include <iostream>
#include <map>
#include <memory>
#include <random>


// ------------------------------------------------------------------------------------------------------------------

int roll_a_fair_dice()
{
    static std::default_random_engine e;
    static std::uniform_int_distribution<int> d{ 1, 6 };

    return d(e);
}


void shuffle_and_display_card_deck()
{
    using card_t = int;
    using deck_t = std::array<card_t, 52>;
    deck_t deck;
    std::iota(std::begin(deck), std::end(deck), card_t{ 0 });

    using engine_t = std::default_random_engine;
    using seed_t   = engine_t::result_type;
    engine_t e{ seed_t(time(0)) }; //a poor seed;

    std::shuffle(std::begin(deck), std::end(deck), e);

    auto suit = [](card_t c) { return "PHKt"[c / 13]; };
    auto rank = [](card_t c) { return "A23456789TJQK"[c % 13]; };
    auto show = [=](card_t c) { std::cout << ' ' << rank(c) << suit(c); };

    std::for_each(std::begin(deck), std::end(deck), show);
}


// ------------------------------------------------------------------------------------------------------------------

// ---------------
// SFINAE
// https://de.wikipedia.org/wiki/Substitution_failure_is_not_an_error
// https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/enable-if
template <typename T> // Von T soll bestimmt werden, ob T::Type ein Typ ist
class HasType {
    // Es werden zwei Typen benötigt, die unterschiedlich groß sind, sodass man sie mit sizeof differenzieren kann:
    typedef char FalseType[1];
    typedef char TrueType[2];

    // Es folgt die überladene Funktion, mittels welcher SFINAE angewendet wird:
    template <typename U>
    static TrueType& Tester(typename U::Type*);

    template <typename>
    static FalseType& Tester(...);

public:
    // Die gesammelte Information wird mit einem einfachen booleschen Wert nach außen gegeben:
    static const bool Value = sizeof(Tester<T>(0)) == sizeof(TrueType);
};


void show_off_sfinae()
{
    struct Foo {
        typedef int Type;
    };

    std::cout << '\n';
    std::cout << std::boolalpha << '\n';
    std::cout << HasType<int>::Value << '\n'; // Gibt false aus
    std::cout << HasType<Foo>::Value << '\n'; // Gibt true aus
}


// ------------------------------------------------------------------------------------------------------------------


// ---------------
// variate_iterator
template <class URBG, class Dist>
class variate_iterator : public std::iterator<std::input_iterator_tag, typename Dist::result_type> {
private:
    using iter_t = variate_iterator;
    using val_t  = typename Dist::result_type;
    using ptr_t  = val_t const*;
    using ref_t  = val_t const&;

    URBG* u{ nullptr }; //non-owning
    Dist* d{ nullptr }; //non-owning
    val_t v{}; // latest variate
    bool valid{ false };

    //help:
    void step() noexcept { valid = false; }

    ref_t deref()
    {
        if (not valid)
            v = (*d)(*u), valid = true;
        return v;
    }

public:
    constexpr variate_iterator() noexcept = default;

    variate_iterator(URBG& u, Dist& d)
            : u{ &u }
            , d{ &d }
    {
    }

    //dereference
    ref_t operator*() { return deref(); }

    ptr_t operator->() { return &deref(); }

    //advance
    iter_t& operator++()
    {
        step();
        return *this;
    }

    iter_t operator++(int)
    {
        iter_t t{ *this };
        step();
        return t;
    }
};

// Anwendung der variate_iterators auf Zufallszahlen
// (liefert bei jedem Aufruf eine neue und kann so zum Füllen eines Vektors mit std::copy_n genutzt werden
void show_off_variate_iterator()
{
    using urbg_t    = std::default_random_engine;
    using variate_t = double;
    using dist_t    = std::uniform_real_distribution<variate_t>;

    urbg_t g{};
    dist_t d{};

    variate_iterator<urbg_t, dist_t> it{ g, d };

    constexpr size_t N = 1000;
    std::vector<variate_t> v(N);
    variate_t val = *it;
    std::copy_n(it, N, std::begin(v));
    val = v[0];
}

// ------------------------------------------------------------------------------------------------------------------


//template normal programming
template <class Element>
struct tree_iterator {
    using supports_plus = std::false_type; // member typedef
    // ...
    tree_iterator& operator++();
};

template <class Element>
struct vector_iterator {
    using supports_plus = std::true_type; // member typedef
    // ...
    vector_iterator& operator++();

    vector_iterator operator+(int);
};

template <class Element>
struct vector {
    using iterator = vector_iterator<Element>; /* ... */
};

template <class Element>
struct set {
    using iterator = tree_iterator<Element>; /* ... */
};


template <class It>
It advance_impl(It begin, int n, std::false_type)
{
    for (int i = 0; i < n; ++i)
        ++begin;
    return begin;
}

template <class It>
It advance_impl(It begin, int n, std::true_type)
{
    return begin + n;
}

template <class Iter>
auto advance(Iter begin, int n)
{
    return advance_impl(begin, n, typename Iter::supports_plus{}); // create an object of that type
}


void show_off_template_iterators()
{
}


// ------------------------------------------------------------------------------------------------------------------

// template CRTP pattern

//definition of class template
template <typename CD>
struct DoubleSpeaker {
    void speaktwice()
    {
        CD* cat_or_dog = static_cast<CD*>(this);
        cat_or_dog->speak();
        cat_or_dog->speak();
    }
};

struct Cat : public DoubleSpeaker<Cat> {
    void speak() { puts("meow"); }
};

struct Dog : public DoubleSpeaker<Dog> {
    void speak() { puts("woof"); }
};


void show_off_CRTP_pattern()
{
    Cat c;
    c.speaktwice();
    Dog d;
    d.speaktwice();
}


// ------------------------------------------------------------------------------------------------------------------

// the Mixin pattern
// see also: http://www.thinkbottomup.com.au/site/blog/C%20%20_Mixins_-_Reuse_through_inheritance_is_good
struct Number {
    using value_type = int;
    int n;

    void set(int v) { n = v; }

    int get() const { return n; }
};

template <typename BASE, typename T = typename BASE::value_type>
struct Undoable : public BASE {
    using value_type = T;
    T before;

    void set(T v)
    {
        before = BASE::get();
        BASE::set(v);
    }

    void undo() { BASE::set(before); }
};

template <typename BASE, typename T = typename BASE::value_type>
struct Redoable : public BASE {
    using value_type = T;
    T after;

    void set(T v)
    {
        after = v;
        BASE::set(v);
    }

    void redo() { BASE::set(after); }
};

using ReUndoableNumber = Redoable<Undoable<Number> >;

void show_off_Mixin_pattern()
{
    ReUndoableNumber mynum;
    mynum.set(42);
    mynum.set(84);
    std::cout << mynum.get() << '\n'; // 84
    mynum.undo();
    std::cout << mynum.get() << '\n'; // 42
    mynum.redo();
    std::cout << mynum.get() << '\n'; // back to 84
}


// ------------------------------------------------------------------------------------------------------------------

// Traits

// first, 2 easy traits examples
// is_void
//template <typename T>
//struct is_void {
//    static constexpr bool value = false;
//};
//template <>
//struct is_void<void> {
//    static constexpr bool value = true;
//};

// modern C++ allows for a shorter form
template <class T>
struct is_void : std::false_type {
};
template <>
struct is_void<void> : std::true_type {
};

// is_pointer
//template <typename T>
//struct is_pointer {
//    static constexpr bool value = false;
//};
//template <typename T>
//struct is_pointer<T*> {
//    static constexpr bool value = true;
//};


// again, shorter form due to modern C++
template <class T>
struct is_pointer : std::false_type {
};
template <class T>
struct is_pointer<T*> : std::true_type {
};


// now the real example
// we have two types of objects (ObjectsA and ObjectB)

class ObjectA {
};

class ObjectB {
public:
    //not necessary as we see below, but this object itslef could have the specialized algorithm as public member functions
    void optimised_implementation()
    {
        std::cout << "Optimized implementation of algorithm for Objects of type 'ObjectB' as a method of ObjectB\n";
    }
};

// again, first the default traits class is created (exactly analogous to is_void above)
template <typename T>
struct supports_optimised_implementation : std::false_type {
};
// and a specialisation of supports_optimised_implementation trait for ObjectB
template <>
struct supports_optimised_implementation<ObjectB> : std::true_type {
};


template <bool b> // does it matter if this param ist bool or int or ...?
struct algorithm_selector {
    template <typename T>
    static void implementation(T& object)
    {
        std::cout << "Standard implementation of algorithm\n";
    }
};

template <>
struct algorithm_selector<true> {
    template <typename T>
    static void implementation(T& object)
    {
        std::cout << "Optimized implementation of algorithm for Objects of type 'ObjectB' exists\n";

        // specialized Implementation could be implemented HERE directly or we can have the object itself take care of that like in this example

        object.optimised_implementation();
    }
};


void show_off_variadic_templates();

// a generic function that the end user of the algorithm will call
// note that it in turn calls algorithm_selector, parameterised using the supports_optimised_implementation traits class:
template <typename T>
void algorithm(T& object)
{
    algorithm_selector<supports_optimised_implementation<T>::value>::implementation(object);
}


void show_off_traits()
{
    bool is_void_1   = is_void<void>::value;
    bool is_void_2   = is_void<void*>::value;
    bool is_pointer1 = is_pointer<void>::value;
    bool is_pointer2 = is_pointer<void*>::value;


    ObjectA a;
    algorithm(a); // calls default implementation
    ObjectB b;
    algorithm(b); // calls ObjectB::optimised_implementation();
}


// ------------------------------------------------------------------------------------------------------------------

// uniform initialization
void show_off_uniform_initialization()
{

    std::vector<std::string> strvec1 = { "string1", "string2" };

    // this does NOT give the same result-> strvec2 will be deduced to be of type std::initializer_list<char const*>
    auto strvec2 = { "string1", "string2" };

    std::map<int, std::string> map1 = { { 1, "One" }, { 2, "Two" } };

    return;
}

// ------------------------------------------------------------------------------------------------------------------


void show_off_move_and_forward()
{

    return;
}


// ------------------------------------------------------------------------------------------------------------------


template <typename T>
void VariadicTemplateFunction(T t)
{
    std::cout << t << '\n';
}

template <typename T, typename... Data>
void VariadicTemplateFunction(T t, Data... d)
{

    std::cout << "# of params remaining: " << sizeof...(d) << " - current value: " << t << '\n';
    VariadicTemplateFunction(d...);
}

void show_off_variadic_templates()
{

    VariadicTemplateFunction(1, 2, 3, 4, 5, 6, 7, 8);

    return;
}


// ------------------------------------------------------------------------------------------------------------------

int main()
{

    //Beispiel (noch refaktorieren)
    //Ein Array mit Würfelergebnissen eines 6-seitigen Würfels füllen
    constexpr int NUMBER_OF_DICE_ROLLS = 1000;
    int dice_rolls[NUMBER_OF_DICE_ROLLS];
    std::generate(dice_rolls, dice_rolls + NUMBER_OF_DICE_ROLLS, roll_a_fair_dice);


    shuffle_and_display_card_deck();

    show_off_variate_iterator();

    show_off_sfinae();

    show_off_template_iterators();

    show_off_CRTP_pattern();
    show_off_Mixin_pattern();


    show_off_traits();


    show_off_uniform_initialization();


    show_off_move_and_forward();


    show_off_variadic_templates();

    return 0;
}
