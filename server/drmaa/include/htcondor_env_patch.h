/*
 ** Copyright 2014-2017 The Earlham Institute
 **
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 **
 **     http://www.apache.org/licenses/LICENSE-2.0
 **
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 */
/*
 * htcondor_patch.h
 *
 *  Created on: 22 Jan 2018
 *      Author: billy
 */

#ifndef CORE_SERVER_DRMAA_INCLUDE_HTCONDOR_ENV_PATCH_H_
#define CORE_SERVER_DRMAA_INCLUDE_HTCONDOR_ENV_PATCH_H_

#ifdef __cplusplus
extern "C" {
#endif

int set_environment_variables (const char *env_vars_s);

int prepare_command (char *buffer_s, const size_t buffer_length, const char * const submit_command_s, const char * const submit_file_name_s);


#ifdef __cplusplus
}
#endif


#endif /* CORE_SERVER_DRMAA_INCLUDE_HTCONDOR_ENV_PATCH_H_ */
