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
    UndoManager(int numStates)
    : indexMax(numStates)
    {
        states.reserve(numStates);
        for (int i = 0; i < indexMax; ++i)
        {
            states.emplace_back(T());
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
        states.at(index).swap(c);
    }
    
    Connector::connectorsCollection& GetPreviousState()
    {
        Decrement();
        if (states.at(index).empty())
        {
            Increment();
        }
        return states.at(index);
    }
    
    Connector::connectorsCollection& GetNextState()
    {
        Increment();
        if (states.at(index).empty())
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
        index-= 1;
        if (index < 0)
        {
            index = indexMax - 1;
        }
    }
    
    const int indexMax;
    std::vector<T> states;
    int index{0};
    bool firstFlag{true};
};
