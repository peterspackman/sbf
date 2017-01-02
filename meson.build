project('sbf', ['c', 'cpp'], 
        version: '0.2.0', default_options: ['c_std=c11', 'cpp_std=c++11'])

conf_data = configuration_data()
conf_data.set('SBF_VERSION_MAJOR', '0')
conf_data.set('SBF_VERSION_MINOR', '2')
conf_data.set('SBF_VERSION_MINOR_MINOR', '0')

inc = include_directories('include')
subdir('tests')
