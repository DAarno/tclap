// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  ArgTraits.h
 *
 *  Copyright (c) 2007, Daniel Aarno, Michael E. Smoot .
 *  Copyright (c) 2017 Google LLC
 *  All rights reserved.
 *
 *  See the file COPYING in the top directory of this distribution for
 *  more information.
 *
 *  THE SOFTWARE IS PROVIDED _AS IS_, WITHOUT WARRANTY OF ANY KIND, EXPRESS
 *  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/

// This is an internal tclap file, you should probably not have to
// include this directly

#ifndef TCLAP_ARG_TRAITS_H
#define TCLAP_ARG_TRAITS_H

namespace TCLAP {

// We use two empty structs to get compile type specialization
// function to work

/**
 * A value like argument value type is a value that can be set using
 * operator>>. This is the default value type.
 */
struct ValueLike {
    using ValueCategory = ValueLike;
    virtual ~ValueLike() = default;
};

/**
 * A string like argument value type is a value that can be set using
 * operator=(string). Useful if the value type contains spaces which
 * will be broken up into individual tokens by operator>>.
 */
struct StringLike {
    virtual ~StringLike() = default;
};

/**
 * A class can inherit from this object to make it have string like
 * traits. This is a compile time thing and does not add any overhead
 * to the inherenting class.
 */
struct StringLikeTrait {
    using ValueCategory = StringLike;
    virtual ~StringLikeTrait() = default;
};

/**
 * A class can inherit from this object to make it have value like
 * traits. This is a compile time thing and does not add any overhead
 * to the inherenting class.
 */
struct ValueLikeTrait {
    using ValueCategory = ValueLike;
    virtual ~ValueLikeTrait() = default;
};

namespace detail {
// Satisfied iff C has a nested C::ValueCategory type.
template <typename C>
concept HasValueCategory = requires { typename C::ValueCategory; };
}  // namespace detail

/**
 * Arg traits are used to get compile type specialization when parsing
 * argument values. Using an ArgTraits you can specify the way that
 * values gets assigned to any particular type during parsing. The two
 * supported types are StringLike and ValueLike. ValueLike is the
 * default and means that operator>> will be used to assign values to
 * the type.
 */
template <typename T>
class ArgTraits {
    // What we want to do is:
    // 1) If there exists a specialization of ArgTraits for type X,
    // use it.
    //
    // 2) If no specialization exists but X has the typename
    // X::ValueCategory, use the specialization for X::ValueCategory.
    //
    // 3) If neither (1) nor (2) defines the trait, use the default
    // which is ValueLike.
    template <typename C, bool = detail::HasValueCategory<C>>
    struct DefaultArgTrait {
        using ValueCategory = ValueLike;
    };

    template <typename C>
    struct DefaultArgTrait<C, true> {
        using ValueCategory = typename C::ValueCategory;
    };

public:
    using ValueCategory = typename DefaultArgTrait<T>::ValueCategory;
};

}  // namespace TCLAP

#endif  // TCLAP_ARG_TRAITS_H
