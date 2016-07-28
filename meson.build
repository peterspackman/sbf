project('sbf', 'c', 
        version: '0.0.1', default_options: ['c_std=c11'])

conf_data = configuration_data()
conf_data.set('SBF_VERSION_MAJOR', '0')
conf_data.set('SBF_VERSION_MINOR', '1')
conf_data.set('SBF_VERSION_MINOR_MINOR', '0')

inc = include_directories('include')
subdir('tests')
