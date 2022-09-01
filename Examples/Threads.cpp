// See full documentation in header

#include "CPPScriptSignal"
#include <chrono> // std::chrono::milliseconds
#include <thread> // std::thread, std::thread::sleep_for
#include <iostream> // std::cout

int main() {
	// Create a new signal named "Welcome" with one parameter
	ScriptSignal<std::string> Welcome;

	// Create a new connection named "Hello" and use the first parameter as "Name"
	Welcome.Connect([](std::string Name) {
		std::cout << "Hello " << Name << '\n';
	});

	// Create a new thread and run it
	std::thread Thread([&] {
		// Sleep for 5000 milliseconds (5 seconds)
		std::this_thread::sleep_for(std::chrono::milliseconds(5000));
		// Note: Welcome will be waiting to be fired

		// After 5 seconds, the Welcome is fired
		Welcome.Fire("Blue");
		// Output: Hello Blue
	}).join();

	// Wait for all functions of connections to be called
	long long Elapsed = Welcome.Wait();

	/** Note:
	 * As the threads are independent of their execution, the thread above sleep
	 * for 5 seconds (don't interrupting Main thread), and signal is already
	 * waiting for this thread to call Welcome::Fire, simultaneously
	 */

	std::cout << "Welcome was fired in " << Elapsed << " milliseconds\n";
	// Output: Welcome was fired in 5000 milliseconds

	return 0;
}
