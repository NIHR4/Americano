#pragma once
#include <type_traits>
#include <utility>

//============================================
// TypeList library
// Original Author: frozenca (https://codereview.stackexchange.com/users/227803/frozenca)
// Original Source: https://codereview.stackexchange.com/questions/269320/c17-typelist-manipulation 
// Modified by AndreNIH (2022)
//============================================
namespace ahl::detail::TL{
    
    template <typename...> struct TypeList;

    template <typename Head, typename... Tails>
    struct TypeList<Head, Tails...> {
        using head = Head;
        using tails = TypeList<Tails...>;
    };

    // length of a typelist

    template <typename TList> struct Length;

    template <typename... Types>
    struct Length<TypeList<Types...>> {
        static constexpr std::size_t value = sizeof...(Types);
    };

    template <typename TList>
    inline constexpr std::size_t Length_v = Length<TList>::value;

    // indexed access

    template <typename TList, std::size_t index> struct TypeAt;

    template <typename Head, typename... Tails>
    struct TypeAt<TypeList<Head, Tails...>, 0> {
        using type = Head;
    };

    template <typename Head, typename... Tails, std::size_t index>
    struct TypeAt<TypeList<Head, Tails...>, index> {
        static_assert(index < sizeof...(Tails) + 1, "index out of range");
        using type = typename TypeAt<TypeList<Tails...>, index - 1>::type;
    };

    template <typename TList, std::size_t index>
    using TypeAt_t = typename TypeAt<TList, index>::type;

    // indexof

    template <typename TList, typename T> struct IndexOf;

    template <typename T>
    struct IndexOf<TypeList<>, T> {
        static constexpr std::size_t value = -1;
    };

    template <typename TList, typename T>
    inline constexpr std::size_t IndexOf_v = IndexOf<TList, T>::value;

    template <typename... Tails, typename T>
    struct IndexOf<TypeList<T, Tails...>, T> {
        static constexpr std::size_t value = 0;
    };

    template <typename Head, typename... Tails, typename T>
    struct IndexOf<TypeList<Head, Tails...>, T> {
        static constexpr std::size_t value = std::is_same_v<Head, T> ? 0 :
                                            (IndexOf_v<TypeList<Tails...>, T> == -1 ? -1 :
                                            IndexOf_v<TypeList<Tails...>, T> + 1);

    };

    // appending to typelists

    template <typename TList1, typename TList2> struct Append;

    template <typename... TList, typename T>
    struct Append<TypeList<TList...>, T> {
        using result_type = TypeList<TList..., T>;
    };

    template <typename T, typename... TList>
    struct Append<T, TypeList<TList...>> {
        using result_type = TypeList<T, TList...>;
    };

    template <typename... TList1, typename... TList2>
    struct Append<TypeList<TList1...>, TypeList<TList2...>> {
        using result_type = TypeList<TList1..., TList2...>;
    };

    template <typename TList1, typename TList2>
    using Append_t = typename Append<TList1, TList2>::result_type;

    // erasing a type from a typelist

    template <typename TList1, typename T> struct Erase;

    template <typename TList1, typename T>
    using Erase_t = typename Erase<TList1, T>::result_type;

    template <typename... Tails, typename T>
    struct Erase<TypeList<T, Tails...>, T> {
        using result_type = TypeList<Tails...>;
    };

    template <typename Head, typename... Tails, typename T>
    struct Erase<TypeList<Head, Tails...>, T> {
        using result_type = Append_t<Head, Erase_t<TypeList<Tails...>, T>>;
    };

    //erasing all matching types from typelist

    template <typename TList1, typename T> struct EraseAll;

    template <typename TList1, typename T>
    using EraseAll_t = typename EraseAll<TList1, T>::result_type;

    template <typename T>
    struct EraseAll<TypeList<>, T> {
        using result_type = TypeList<>;
    };

    template <typename... Tails, typename T>
    struct EraseAll<TypeList<T, Tails...>, T> {
        using result_type = EraseAll_t<TypeList<Tails...>, T>;
    };

    template <typename Head, typename... Tails, typename T>
    struct EraseAll<TypeList<Head, Tails...>, T> {
        using result_type = Append_t<Head, EraseAll_t<TypeList<Tails...>, T>>;
    };

    //erasing all  types from typelist from [0,N] 

    template<typename TList, std::size_t N> 
    struct EraseN;

    template<typename TList, std::size_t N>
    using EraseN_t = typename EraseN<TList, N>::result_type;

    template<std::size_t N>
    struct EraseN<TypeList<>, N>{
        using result_type = TypeList<>;
    };
    
    //Eraser specialization
    template<typename Head, typename... Tails, std::size_t N>
    struct EraseN<TypeList<Head, Tails...>,N>{
        using result_type =  EraseN_t<TypeList<Tails...>, N-1>;
    };

    template<typename Head, typename... Tails>
    struct EraseN<TypeList<Head, Tails...>, 0>{
        using result_type =  TypeList<Head,Tails...>;
    };

    //========================================

    template<typename TList, std::size_t N, typename=void>
    struct EraseFloatsUpToN;
    
    template<typename TList, std::size_t N>
    using EraseFloatsUpToN_t = typename EraseFloatsUpToN<TList, N>::result_type;

    //Specialize in case typelist gets depleated before reaching the max ammount of items
    template<std::size_t N>
    struct EraseFloatsUpToN<TypeList<>, N> {
        using result_type = TypeList<>;
    };
    
    //Eraser specialization
    template<typename... Tails, std::size_t N>
    struct EraseFloatsUpToN<TypeList<double, Tails...>, N>{
        using result_type = EraseFloatsUpToN_t<TypeList<Tails...>, N-1>;
    };

    template<typename... Tails, std::size_t N>
    struct EraseFloatsUpToN<TypeList<float, Tails...>, N>{
        using result_type = EraseFloatsUpToN_t<TypeList<Tails...>, N-1>;
    };


    //Specialization in case of hitting the max ammount of elements to ersase
    template<typename... Tails, typename T>
    struct EraseFloatsUpToN<TypeList<T, Tails...>, 0>{
        using result_type = TypeList<T,Tails...>;
    };
    
    //These two specializations are ugly but I couldn't get this to work otherwise
    //Specialization in case of hitting the max ammount of elements to ersase
    template<typename... Tails>
    struct EraseFloatsUpToN<TypeList<float, Tails...>, 0>{
        using result_type = TypeList<float,Tails...>;
    };
    //Specialization in case of hitting the max ammount of elements to ersase
    template<typename... Tails>
    struct EraseFloatsUpToN<TypeList<double, Tails...>, 0>{
        using result_type = TypeList<double,Tails...>;
    };

    template<typename Head, typename... Tails, std::size_t N>
    struct EraseFloatsUpToN<TypeList<Head, Tails...>, N>{
        //using result_type = typename Append_t<Head, EraseFloatsUpToN_t<TypeList<Tails...>, N-1>>;
        using result_type =  Append_t<Head, typename EraseFloatsUpToN<TypeList<Tails...>, N-1 >::result_type>;
    };

    //===============================================
    template<typename Elem, bool Check>
    struct EraseIfImpl;

    template<typename T, bool Check>
    using EraseIfImpl_t = typename EraseIfImpl<T, Check>::result_type;

    template<typename Elem>
    struct EraseIfImpl<Elem,  true>{
        using result_type = TypeList<>;
    };

    template<typename Elem>
    struct EraseIfImpl<Elem,  false>{
        using result_type = TypeList<Elem>;
    };


    template<typename TList, template<typename> typename Pred>
    struct EraseIf;

    template<typename TList, template<typename> typename Pred>
    using EraseIf_t = typename EraseIf<TList, Pred>::result_type;

    template<template<typename> typename Pred>
    struct EraseIf<TypeList<>, Pred>{
        using result_type = TypeList<>;
    };

    template<typename Head, typename... Tail, template<typename> typename Pred>
    struct EraseIf<TypeList<Head,Tail...>, Pred>{
        using result_type = Append_t<EraseIfImpl_t<Head, Pred<Head>::value>, EraseIf_t<TypeList<Tail...>, Pred>>;
    };
    //=====================================
    
    //Create subrange of lists
    template<typename TList, std::size_t N>
    struct ExtractSublist;
    
    template<typename TList, std::size_t N>
    using ExtractSublist_t =  typename ExtractSublist<TList, N>::type;
    
    template<typename Head, typename... Tails, std::size_t N>
    struct ExtractSublist<TypeList<Head, Tails...>, N>{
        using type = Append_t<Head, typename ExtractSublist<TypeList<Tails...>, N-1>::type >;
    };

    template<typename Head, typename... Tails>
    struct ExtractSublist<TypeList<Head, Tails...>, 0>{
        using type = TypeList<>;
    };

    template<std::size_t N>
    struct ExtractSublist<TypeList<>, N>{
        using type = TypeList<>;
    };


    //Count occurances of type in TypeList
    template<typename TList, typename T>
    struct CountTypeInList;

    template<typename TList, typename T>
    constexpr int CountTypeInList_v = CountTypeInList<TList,T>::value;

    template<typename Head, typename... Tail, typename T>
    struct CountTypeInList<TypeList<Head, Tail...>, T>{
        static constexpr int value = CountTypeInList_v<TypeList<Tail...>, T >;
    };
    
    template<typename... Tail, typename T>
    struct CountTypeInList<TypeList<T, Tail...>, T>{
        static constexpr int value =  1+CountTypeInList_v<TypeList<Tail...>, T> ;
    };
    
    template<typename T>
    struct CountTypeInList<TypeList<>, T>{
        static constexpr int value = 0;
    };

    //Count-If
    template<template<typename> typename Pred, typename TList>
    struct CountElementsIf;
    
    template<template<typename> typename Pred, typename TList>
    constexpr int CountElementsIf_v = CountElementsIf<Pred,TList>::value;

    template<template<typename> typename Pred>
    struct CountElementsIf<Pred, TypeList<>>{
        static constexpr int value=0;
    };

    template<template<typename> typename Pred, typename Head, typename... Tail>
    struct CountElementsIf<Pred, TypeList<Head,Tail...>>{
        static constexpr int value = Pred<Head>::value + CountElementsIf_v<Pred, TypeList<Tail...>>;
    };

    //ReplaceTypeAt
    template<typename TList, std::size_t index, typename ReplaceT>
    struct ReplaceTypeAt;

    template<typename TList, std::size_t index, typename ReplaceT>
    using ReplaceTypeAt_t = typename ReplaceTypeAt<TList, index, ReplaceT>::result_type;

    template<typename Head, typename... Tail, std::size_t index, typename ReplaceT>
    struct ReplaceTypeAt<TypeList<Head, Tail...>, index, ReplaceT>{
        static_assert(index < sizeof...(Tail) + 1, "index out of range");
        using result_type = Append_t<Head, ReplaceTypeAt_t<TypeList<Tail...>, index-1, ReplaceT>>;
    };

    template<typename Head, typename... Tail, typename ReplaceT>
    struct ReplaceTypeAt<TypeList<Head, Tail...>, 0u, ReplaceT>{
        using result_type = Append_t<ReplaceT, TypeList<Tail...>>;
    };

    //Insert Type At
    template<typename TList, std::size_t index, typename T>
    struct InsertTypeAt;

    template<typename TList, std::size_t index, typename T>
    using InsertTypeAt_t = typename InsertTypeAt<TList, index, T>::result_type;

    template<typename Head, typename... Tail, std::size_t index, typename T>
    struct InsertTypeAt<TypeList<Head,Tail...>, index, T>{
        static_assert(index < sizeof...(Tail) + 1, "index out of range");
        using result_type = Append_t<Head, InsertTypeAt_t<TypeList<Tail...>,index-1,T>>;
    };


    template<typename Head, typename... Tail, typename T>
    struct InsertTypeAt<TypeList<Head,Tail...>, 0, T>{
        using result_type = Append_t<T,TypeList<Head,Tail...>>;
    };

}
//Additional TypeList helpers
namespace ahl::detail{
    template<typename R, typename TList, typename Is>
    struct TListToFnImpl;

    template<typename R, typename TList, typename Is>
    using TListToFnImpl_t = typename TListToFnImpl<R,TList, Is>::type;

    template<typename R,typename TList, template<typename I, I...> typename Is, std::size_t... Ix>
    struct TListToFnImpl<R,TList, Is<std::size_t, Ix...> >{
        using type =  R(TL::TypeAt_t<TList, Ix>...);
    };
    
    //non mem-fn
    template<typename R, typename TList>
    struct TypeListToFn{
        using type = TListToFnImpl_t<R, TList, std::make_index_sequence<TL::Length_v<TList>>>; 
    };

    template<typename R, typename TList>
    using TypeListToFn_t = typename TypeListToFn<R,TList>::type;
    
    //mem-fn
    template<typename R, typename T, typename TList, typename Is>
    struct TListToMemFnImpl;

    template<typename R, typename T, typename TList, typename Is>
    using TListToMemFnImpl_t = typename TListToMemFnImpl<R, T,TList, Is>::type;

    template<typename R, typename T, typename TList, template<typename I, I...> typename Is, std::size_t... Ix>
    struct TListToMemFnImpl<R, T, TList, Is<std::size_t, Ix...> >{
        using type = R(T::*)(TL::TypeAt_t<TList, Ix>...);
    };

    template<typename R, typename T,typename TList>
    struct TypeListToMemFn{
        using type = TListToMemFnImpl_t<R, T, TList, std::make_index_sequence<TL::Length_v<TList>>>; 
    };

    template<typename R, typename T, typename TList>
    using TypeListToMemFn_t = typename TypeListToMemFn<R, T, TList>::type;



    template<typename TList, std::size_t index>
    constexpr std::size_t CountEllidedTypesAtPos = std::is_floating_point_v<TL::TypeAt_t<TList, index>> + CountEllidedTypesAtPos<TList,index-1>;
    
    template<typename TList>
    constexpr std::size_t CountEllidedTypesAtPos<TList,0> = std::is_floating_point_v<TL::TypeAt_t<TList,0>>;


    

    
}