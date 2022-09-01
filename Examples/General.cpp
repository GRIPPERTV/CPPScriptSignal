// See full documentation in header

#include "CPPScriptSignal"
#include <string> // std::string
#include <iostream> // std::cout

int main() {
	// Create a new signal named "Welcome" with one parameter
	ScriptSignal<std::string> Welcome;

	// Create a new connection named "Hello" and use the first parameter as "Name"
	auto Hello = Welcome.Connect([](std::string Name) {
		std::cout << "Hello " << Name << '\n';
	});

	// All connections of Welcome will receive the argument "Blue"
	Welcome.Fire("Blue");
	// Output: Hello Blue

	// Verify if Hello's function still exist
	std::cout << Hello->Connected() << '\n';
	// Output: 1

	// After disconnect, the Hello's function don't exist anymore
	Hello->Disconnect();

	// Now the connection don't have any function to use
	std::cout << Hello->Connected() << '\n';
	// Output: 0

	/* Now none of connections will receive this argument,
	because the single connection created don't have anymore a function */
	Welcome.Fire("Purple");

	return 0;
}
