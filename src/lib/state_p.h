/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>
    SPDX-FileCopyrightText: 2018 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KSYNTAXHIGHLIGHTING_STATE_P_H
#define KSYNTAXHIGHLIGHTING_STATE_P_H

#include "state.h"
#include <vector>

#include <QSharedData>
#include <QStringList>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
using qhash_result_t = uint;

// copying from QT6 source code for supporting QT5's qHashMulti
namespace QtPrivate {
template<typename T>
inline constexpr bool QNothrowHashableHelper_v = noexcept(qHash(std::declval<const T &>()));

template<typename T, typename Enable = void>
struct QNothrowHashable : std::false_type
{};

template<typename T>
struct QNothrowHashable<T, std::enable_if_t<QNothrowHashableHelper_v<T>>> : std::true_type
{};

template<typename T>
constexpr inline bool QNothrowHashable_v = QNothrowHashable<T>::value;

} // namespace QtPrivate

template<typename... T>
constexpr qhash_result_t qHashMulti(qhash_result_t seed, const T &...args) noexcept(
    std::conjunction_v<QtPrivate::QNothrowHashable<T>...>)
{
    QtPrivate::QHashCombine hash;
    return ((seed = hash(seed, args)), ...), seed;
}
#else
using qhash_result_t = size_t;
#endif

namespace KSyntaxHighlighting {
class Context;

class StateData : public QSharedData
{
    friend class State;
    friend class AbstractHighlighter;
    friend std::size_t qHash(const StateData &, std::size_t);

public:
    StateData() = default;

    static StateData *reset(State &state);
    static StateData *detach(State &state);

    static StateData *get(const State &state) { return state.d.data(); }

    std::size_t size() const { return m_contextStack.size(); }

    void push(Context *context, QStringList &&captures);

    /**
     * Pop the number of elements given from the top of the current stack.
     * Will not pop the initial element.
     * @param popCount number of elements to pop
     * @return false if one has tried to pop the initial context, else true
     */
    bool pop(int popCount);

    Context *topContext() const { return m_contextStack.back().context; }

    const QStringList &topCaptures() const { return m_contextStack.back().captures; }

    struct StackValue
    {
        Context *context;
        QStringList captures;

        bool operator==(const StackValue &other) const
        {
            return context == other.context && captures == other.captures;
        }
    };

private:
    /**
     * definition id to filter out invalid states
     */
    uint64_t m_defId = 0;

    /**
     * the context stack combines the active context + valid captures
     */
    std::vector<StackValue> m_contextStack;
};

inline std::size_t qHash(const StateData::StackValue &stackValue, std::size_t seed = 0)
{
    return qHashMulti(seed, stackValue.context, stackValue.captures);
}

inline std::size_t qHash(const StateData &k, std::size_t seed = 0)
{
    return qHashMulti(
        seed, k.m_defId, qHashRange(k.m_contextStack.begin(), k.m_contextStack.end(), seed));
}
} // namespace KSyntaxHighlighting

#endif
