/*
  ==============================================================================

    SOSUndoManager.h
    Created: 5 Jul 2023 11:58:29am
    Author:  Mason Self

  ==============================================================================
*/
#pragma once

template <typename T>
struct UndoManager
{
    //TODO: add initial state upon construction
    //Presently the first state stored is the first CHANGE and the initial state can not be undo'd to
    explicit UndoManager(size_t numStates)
    : indexMax(numStates)
    {
        for (auto i = 0; i < numStates; ++i)
        {
            states.emplace_back(std::optional<T>());
        }
    }
    
    void AddState(T c)
    {
        if (! firstFlag)
        {
            Increment();
        }
        else
        {
            firstFlag = false;
        }
            states.at(index) = std::optional<T>(c);
    }
    
    std::optional<T>& GetPreviousState()
    {
        Decrement();
        if (! states.at(index).has_value())
        {
            Increment();
        }
        return states.at(index);
    }
    
    std::optional<T>& GetNextState()
    {
        Increment();
        if (! states.at(index).has_value())
        {
            Decrement();
        }
        return states.at(index);
    }
    
    void Increment()
    {
        ++index;
        if (index == indexMax)
        {
            index = 0;
        }
    }
    
    void Decrement()
    {
        if (index == 0)
        {
            index = indexMax - 1;
            return;
        }
        index-= 1;
    }
    
    const size_t indexMax;
    std::vector< std::optional<T> > states;
    size_t index{0};
    bool firstFlag{true};
};
