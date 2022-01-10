#pragma once

void ResetCounters();

struct Arg
{
    static int copy_counter;
    static int move_counter;

    Arg() = default;
    Arg(const Arg& r)
    {
        ++copy_counter;
    }

    Arg(Arg&& r)
    {
        ++move_counter;
    }

    Arg& operator = (const Arg&)
    {
        ++copy_counter;
    }

    Arg& operator = (Arg&&)
    {
        ++move_counter;
    }
};

struct Result
{
    static int copy_counter;
    static int move_counter;

    Result(int id = 0):
        m_id(id)
    {
    }

    Result(const Result& r)
    {
        ++copy_counter;
        m_id = r.m_id;
    }

    Result& operator = (const Result& r)
    {
        ++copy_counter;
        m_id = r.m_id;
        return *this;
    }

    Result(Result&& r)
    {
        ++move_counter;
        m_id = r.m_id;
    }

    Result& operator = (Result&& r)
    {
        ++move_counter;
        m_id = r.m_id;
        return *this;
    };

    int m_id;
};

inline bool operator == (const Result& left, const Result& right)
{
    return left.m_id == right.m_id;
}

