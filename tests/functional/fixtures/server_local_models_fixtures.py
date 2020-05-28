#
# Copyright (c) 2019 Intel Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import shutil
import os
import pytest

import config
from model.models_information import Resnet, AgeGender
from utils.server import start_ovms_container, save_container_logs


@pytest.fixture(scope="class")
def start_server_single_model(request, get_docker_context):

    def finalizer():
        save_container_logs(container=container)
        container.stop()

    request.addfinalizer(finalizer)

    start_server_command_args = {"model_name": Resnet.name,
                                 "model_path": Resnet.model_path,
                                 "plugin_config": "\"{\\\"CPU_THROUGHPUT_STREAMS\\\": \\\"CPU_THROUGHPUT_AUTO\\\"}\""}
    container_name_infix = "test-single"

    # In this case, slower, non-default serialization method is used
    env_variables = ['SERIALIZATON=_prepare_output_as_AppendArrayToTensorProto']

    container, ports = start_ovms_container(get_docker_context, start_server_command_args,
                                            container_name_infix, config.start_container_command, env_variables)

    return container, ports


@pytest.fixture(scope="class")
def start_server_with_mapping(request, get_docker_context):

    def finalizer():
        save_container_logs(container=container)
        container.stop()

        if os.path.exists(file_dst_path):
            os.remove(file_dst_path)

    request.addfinalizer(finalizer)

    file_dst_path = config.path_to_mount + '/age_gender/1/mapping_config.json'
    shutil.copyfile('tests/functional/mapping_config.json', file_dst_path)

    start_server_command_args = {"model_name": AgeGender.name,
                                 "model_path": AgeGender.model_path}
    container_name_infix = "test-2-out"
    container, ports = start_ovms_container(get_docker_context, start_server_command_args,
                                            container_name_infix, config.start_container_command)

    def delete_mapping_file():
        if os.path.exists(file_dst_path):
            os.remove(file_dst_path)

    request.addfinalizer(delete_mapping_file)

    return container, ports
