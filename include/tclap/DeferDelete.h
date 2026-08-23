// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  DeferDelete.h
 *
 *  Copyright (c) 2020, Google LLC
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

#ifndef TCLAP_DEFER_DELETE_H
#define TCLAP_DEFER_DELETE_H

#include <memory>
#include <vector>

namespace TCLAP {

/**
 * DeferDelete can be used by objects that need to allocate arbitrary other
 * objects to live for the duration of the first object. Any object
 * added to DeferDelete (by calling operator()) will be deleted when
 * the DeferDelete object is destroyed.
 */
class DeferDelete {
    class DeletableBase {
    public:
        virtual ~DeletableBase() = default;
    };

    template <typename T>
    class Deletable : public DeletableBase {
    public:
        Deletable(T *o) : _o(o) {}
        ~Deletable() override { delete _o; }

        Deletable(const Deletable<T> &) = delete;
        Deletable<T> &operator=(const Deletable<T> &) = delete;

    private:
        T *_o;
    };

    std::vector<std::unique_ptr<DeletableBase>> _toBeDeleted;

public:
    DeferDelete() : _toBeDeleted() {}

    template <typename T>
    void operator()(T *toDelete) {
        _toBeDeleted.push_back(std::make_unique<Deletable<T>>(toDelete));
    }
};

}  // namespace TCLAP

#endif  // TCLAP_DEFER_DELETE_H
