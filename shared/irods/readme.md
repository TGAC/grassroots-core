# iRODS {#irods_library}

[iRODS](www.irods.org) is a system of storing files with associated metadata within a filesystem. iRODS instances that are located disparately can be shared with each other to provide access to shared resources. 
Part of the Grassroots system provides a wrapper around sets of iRods functions to give a higher level API. 
The Grassroots iRODS library currently adds functionality in two areas. 
The first is to get information about the data objects that a user owns, such as all data objects within a collection, any objects modified between two dates, *etc.* 
The second is to allow for more refined searches of both the iCAT metadata catalogue and also the other data areas within the iRODS system. 
The functionality in this library is also used for the [iRODS Search Service](@ref irods_search_service).

 
