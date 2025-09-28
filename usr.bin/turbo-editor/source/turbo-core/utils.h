#ifndef TURBO_UTILS_H
#define TURBO_UTILS_H

#define Uses_TDialog
#define Uses_TProgram
#define Uses_TFileDialog
#include <tvision/tv.h>

#include <unordered_map>

template <class Func>
// 'callback' should take a TView * and return something evaluable to a bool.
inline ushort execDialog(TDialog *d, void *data, Func &&callback)
{
    TView *p = TProgram::application->validView(d);
    if (p) {
        if (data)
            p->setData(data);
        ushort result;
        do {
            result = TProgram::application->execView(p);
        } while (result != cmCancel && !callback(p));
        TObject::destroy(p);
        return result;
    }
    return cmCancel;
}

template <typename Func>
inline void openFileDialog( TStringView aWildCard, TStringView aTitle,
                            TStringView inputName, ushort aOptions,
                            uchar histId, Func &&callback )
{
    auto *dialog = new TFileDialog( aWildCard, aTitle,
                                    inputName, aOptions,
                                    histId );
    execDialog(dialog, nullptr, std::move(callback));
}

template<typename K, typename V>
class const_unordered_map : public std::unordered_map<K, V>
{
public:

    using super = std::unordered_map<K, V>;
    using super::super;

    V operator[](const K &key) const {
        auto it = super::find(key);
        if (it == super::end())
            return V {};
        return it->second;
    }

};

#endif // TURBO_UTILS_H
