# python/after_effects_plugin/parameter_classes.py

class Parameter:
    def __init__(self, name, param_type, default, param_id, **kwargs):
        self.name = name
        self.type = param_type
        self.default = default
        self.id = param_id
        self.kwargs = kwargs

class Slider(Parameter):
    def __init__(self, name, default, min_val, max_val, param_id, i_resize_buffer=False):
        super().__init__(name, "slider", default, param_id, min=min_val, max=max_val, i_resize_buffer=i_resize_buffer)
        #if i_resize_buffer is true, need to add CMD_Frame_Setup, and use the factor to resize. (factor must not require any other changes)

class Checkbox(Parameter):
    def __init__(self, name, default, param_id):
        super().__init__(name, "checkbox", default, param_id)

class Color(Parameter):
    def __init__(self, name, default, param_id):
        super().__init__(name, "color", default, param_id)

class Point(Parameter):
    def __init__(self, name, default, param_id):
        super().__init__(name, "point", default, param_id)

class Point3D(Parameter):
    def __init__(self, name, default, param_id):
        super().__init__(name, "point3d", default, param_id)

class Popup(Parameter):
    def __init__(self, name, choices, default, param_id):
        super().__init__(name, "popup", default, param_id, choices=choices)
