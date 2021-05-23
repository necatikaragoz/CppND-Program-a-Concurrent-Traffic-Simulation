#include <iostream>
#include <random>
#include <chrono>

#include "TrafficLight.h"
#include <queue>


/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    
    // perform queue modification under the lock
    std::unique_lock<std::mutex> uLock(_mutex);

    _cond.wait(uLock, [this] { return !_messages.empty(); }); // pass unique lock to condition variable

    // remove last vector element from queue
    T msg = std::move(_messages.back());
    _messages.pop_back();

    return msg; // will not be copied due to return value optimization (RVO) in C++

}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.

    // simulate some work
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // perform vector modification under the lock
    std::lock_guard<std::mutex> uLock(_mutex);

    // add vector to queue
    std::cout << "   Message " << msg << " has been sent to the queue" << std::endl;
    _messages.push_back(std::move(msg));
    _cond.notify_one(); // notify client after pushing new Vehicle into vector
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
    _type = ObjectType::objectTrafficLight;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while (true) {
        TrafficLightPhase phase = _messageQueue.receive();
        if (phase == TrafficLightPhase::green)
            return;
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}


// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 

    auto begin  =  std::chrono::steady_clock::now();  //std::chrono::steady_clock::now();
     
    // random number generate
    int randomWaitMs = ( ( rand() % 3 ) + 4) ; // randomNum in the range 4 to 6

    //std::cout << "randomWaitMs = " << randomWaitMs << std::endl;
    
    //Check the time 
    while(true)
    {
        // wait with 1 ms resolution
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        std::chrono::duration<double> elapsed = std::chrono::steady_clock::now() - begin;

        // if the tiem is excited then change the mode
        if(elapsed.count() >= randomWaitMs)
        {
            //std::cout << "elapsedtime = " << elapsed.count() <<  std::endl;
            // update time 
            begin = std::chrono::steady_clock::now();
            
            //update phase and send message
            UpdatePhase();
            
        }
    }
}



void TrafficLight::UpdatePhase()
{
    TrafficLightPhase phase = TrafficLightPhase::green;
    /* Toggle current phase of traffic light */
    if (_currentPhase == TrafficLightPhase::green)
    {
        phase = TrafficLightPhase::red;
    }

   // std::cout << "phase = " << phase << std::endl;
    
    _currentPhase = phase;

    _messageQueue.send(std::move(phase) );
}

