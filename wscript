# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

# def options(opt):
#     pass

# def configure(conf):
#     conf.check_nonfatal(header_name='stdint.h', define_name='HAVE_STDINT_H')

def build(bld):
    module = bld.create_ns3_module('ospf', ['core','internet'])
    module.source = [
        'model/ipv4-ospf-routing.cc',
        'model/conf-loader.cc',
        'helper/ipv4-ospf-routing-helper.cc',
        'util/subnet.cc',
        ]

    module_test = bld.create_ns3_module_test_library('ospf')
    module_test.source = [
        'test/ospf-test-suite.cc',
        ]

    headers = bld(features='ns3header')
    headers.module = 'ospf'
    headers.source = [
        'model/ipv4-ospf-routing.h',
        'model/conf-loader.h',
        'helper/ipv4-ospf-routing-helper.h',
        'util/subnet.h',
        ]

    if bld.env.ENABLE_EXAMPLES:
        bld.recurse('examples')

    # bld.ns3_python_bindings()

