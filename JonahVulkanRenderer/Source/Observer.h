#pragma once

class IObserver {

public:
	virtual void ObserverUpdate(float a, float b) = 0;
};