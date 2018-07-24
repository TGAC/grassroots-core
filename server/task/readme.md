# Grassroots Task Library {#task_library_guide}

This library provides an API for running and interacting with asynchronous tasks within the Grassroots Infrastructure. 
Typically it uses threads so an understanding of the various issues with these is very useful.
It provides an API so that the developer does not need to worry about the underlying platform-specific methods.

This is useful when you have tasks that take a long time to tun and you do not want 
the user's session to be left in a waiting state whilst the task runs. By using the Task library, control can be passed straight back to the user and they can check on the 
progress of the tasks as they see fit. The [Asynchronous Services](@ref async_services_guide) documentation provides a lot of the neccessary information and background needed.
 

## Lifecycle

This library uses the principle of sending signals between producer and consumer objects with these objects. The producer objects are defined in the AsyncTask datatype and its child classes such as CountAsyncTask or SystemAsyncTask. 

The work that you wish to carry out asynchronously is done using an AsyncTask object. 
This object is given a function that will be ran asynchronously by using its SetAsyncTaskRunData() function and it is then started by calling RunAsyncTask().

When an AsyncTask runs, it sends signals as it completes various stages of its workflow as defined by the Service running them. 

A CountAsyncTask has an internal counter that gets incremented using IncrementCountAsyncTask() and when this counter reaches its defined limit it will then send a signal to any interested consumer objects.

A SystemAsyncTask runs a given command using the system() function and notifies any consumers of the success of this when this has completed.

The typical workflow of a Service means that it is only guaranteed to be available whilst a request from a user is being processed. Once the response has been sent back the Service will usually be freed. 
However when a System has ServiceJobs that are running AsyncTasks, this would not work so instead the Service is made hidden and freed when all of its AsyncTasks have completed.
So that the status of these AsyncTasks can be queried and any results retrieved, the ServiceJobs need to store the required data, so they are registered with the global JobsManager so that they are persistent.

