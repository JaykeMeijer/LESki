app.directive('ngRightClick', function($parse) {
    return function(scope, element, attrs) {
        var fn = $parse(attrs.ngRightClick);
        element.bind('contextmenu', function(event) {
            scope.$apply(function() {
                event.preventDefault();
                fn(scope, {$event:event});
            });
        });
    };
});

app.controller('AppController', function AppController($scope, $timeout, ngDialog) {
    var ac = this;
    ac.loaded = false;

    $scope.leds = [[], [] ,[] ,[]];
    ac.active_color = "#ffffff"
    ac.percentage = 100;

    $scope.new_sequence = function() {
        $scope.new_color_mode = true;
        var dialog = ngDialog.open({
            template: "templates/new_sequence.html",
            className: 'ngdialog-theme-default',
            scope: $scope
        });
    }

    $scope.create_new_sequence = function(colormode) {
        new_sequence(colormode);
        if (!colormode) {
            ac.active_color = "#ffffff";
        }
        $scope.create_new_frame(0);
    }

    function new_sequence(colormode) {
        $scope.sequence = {
            color: colormode,
            simulating: false,
            frames: [],
            active_frame: 0,
            steptime: 1000
        };
    }

    function init() {
        // Initialize the application
        var awaiting = 0;

        // Set key commands
        $(document).keydown(function(e) {
            if ($(e.target).closest("input")[0]) {
                return;
            }
            console.log(e.keyCode)
            switch(e.keyCode) {
                case 37:
                    if ($scope.sequence.frames.length > 1) {
                        $scope.select_frame($scope.sequence.active_frame - 1);
                    }
                    break;
                case 39:
                    if ($scope.sequence.frames.length > 1) {
                        $scope.select_frame($scope.sequence.active_frame + 1);
                    }
                    break;
                case 107:
                case 187:
                    $scope.create_new_frame($scope.sequence.active_frame + 1, $scope.leds);
                    break;
                case 109:
                case 189:
                case 46:
                    if ($scope.sequence.frames.length > 1) {
                        $scope.remove_frame($scope.sequence.active_frame);
                    }
                    break;
                case 13:
                case 32:
                    $scope.toggle_simulate();
                    break;
                case 38:
                    $scope.sequence.steptime += 10;
                    break;
                case 40:
                    $scope.sequence.steptime -= 10;
                    break;
            }
            $timeout();
        });

        callback();

        function callback() {
            awaiting--;
            if (awaiting < 1) {
                ac.loaded = true;
            }
        }
    }
    init();

    $scope.initSlider = function() {
        var el = document.getElementById('brightness_slider');
        var el2 = document.getElementById('brightness');
        ac.slider = noUiSlider.create(el, {
            start: [100],
            step: 1,
            range: {
                'min': [0],
                'max': [100]
            }
        });
        ac.slider.on('change', function(){
            $scope.set_percentage(ac.slider.get());
        });
        ac.slider.on('update', function( values, handle ) {
            el2.innerHTML = Math.round(values[handle]);
        });
    }

    $scope.store_file = function() {
        var dialog = ngDialog.open({
            template: 'templates/prompt_filename.html',
            className: 'ngdialog-theme-default',
            scope: $scope
        });
    }

    $scope.load_file = function() {
        var dialog = ngDialog.open({
            template: 'templates/prompt_filename_load.html',
            className: 'ngdialog-theme-default',
            scope: $scope
        });
    }

    $scope.write_file = function(filename) {
        /* Data model:
        * <colormode><number of frames>\n
        * <frame>\n
        * <frame>\n
        * <frame>\n
        * 
        * Frame for color mode:
        * <row1-led1-R><row1-led1-G><row1-led1-B><row1-led2-R>...<row1-led25-G><row1-led25-B>\t
        * <row2-led1-R><row2-led1-G><row2-led1-B><row2-led2-R>...<row2-led25-G><row2-led25-B>\t
        * etc.
        * 
        * Frame for non-color mode:
        * <row1-led1><row1-led2>...<row1-led25>\t
        * <row1-led1><row1-led2>...<row1-led25>\t
        * <row1-led1><row1-led2>...<row1-led25>\t
        */

        // Determine required size
        // rows per frame:
        var rows = 4;

        // LEDS per row
        var leds = 25;

        size = 3; // Metadata
        if ($scope.sequence.color) {
            size += $scope.sequence.frames.length * (rows * (leds * 3 + 1) + 1);
        } else {
            size += $scope.sequence.frames.length * (rows * (leds + 1) + 1);
        }
        console.log(size);

        // Generate metadata
        data = new Uint8Array(size);
        datapointer = 0;
        writeByte($scope.sequence.color ? 1 : 0);
        writeByte($scope.sequence.frames.length);
        writeByte(10);

        for (var i = 0; i < $scope.sequence.frames.length; i++) {
            var frame = $scope.sequence.frames[i];
            for (var j = 0; j < frame.length; j++) {
                var row = frame[j];
                for (var k = 0; k < row.length; k++) {
                    val = htmlToRGB(row[k].color)
                    if ($scope.sequence.color) {
                        writeByte(val.r);
                        writeByte(val.g);
                        writeByte(val.b);
                    } else {
                        writeByte(val.r);
                    }
                    
                }
                writeByte(11); // \t
            }
            writeByte(10); // \n
        }
        
        var fs = require('fs');
        fs.writeFileSync("sequences/" + filename, data);

        function writeByte(byte) {
            data[datapointer] = byte;
            datapointer++;
        }
    }

    $scope.read_file = function(filename) {
        var fs = require('fs');
        try {
            data = fs.readFileSync("sequences/" + filename);
        } catch (err) {
            alert('Failed to read that file');
            return;
        }
        var datapointer = 0;
        new_sequence();

        // Parse metadata
        $scope.sequence.color = (readByte() == 1);
        console.log('Sequence contains ' + readByte() + ' frames');
        $scope.sequence.frames = [];
        
        // Ensure metadata is finished
        var b = readByte();
        while (b != 10) {
            console.log('Found unused metadata byte: ' + b);
            b = readByte();
        }

        // Now real data starts
        b = readByte();
        var framepointer = 0;
        while (b != undefined) {
            // Add new frame
            $scope.sequence.frames.push([]);

            var frame = $scope.sequence.frames[framepointer];
            var rowpointer = 0;
            while (b != 10) {
                // Add new row
                frame.push([]);
                var row = frame[rowpointer];
                var ledpointer = 0;
                while (b != 11) {
                    row.push([])
                    if ($scope.sequence.color) {
                        var rgb_r = b;
                        var rgb_g = readByte();
                        var rgb_b = readByte();
                        row[ledpointer] = {color: rgbToHex(rgb_r, rgb_b, rgb_g)};
                    } else {
                        row[ledpointer] = {color: rgbToHex(b, b, b)};
                    }
                    ledpointer++;
                    b = readByte();
                }
                // New row starting now
                rowpointer++;
                b = readByte();
            }
            // New frame starting now
            framepointer++;
            b = readByte();
        }

        // Load complete, update screen
        $scope.select_frame(0);

        function readByte() {
            var byte = data[datapointer];
            datapointer++;
            return byte;
        }
    }

    function htmlToRGB(hex) {
        var result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(hex);
        return result ? {
            r: parseInt(result[1], 16),
            g: parseInt(result[2], 16),
            b: parseInt(result[3], 16)
        } : null;
    }

    function rgbToHex(r, g, b) {
        console.log('Converting ' + r + ', ' + g + ', ' + b)
        function componentToHex(c) {
            var hex = c.toString(16);
            return hex.length == 1 ? "0" + hex : hex;
        }
        return '#' + componentToHex(r) + componentToHex(g) + componentToHex(b);
    }

    $scope.create_new_frame = function(position, based_on) {
        if (position >= $scope.sequence.frames.length) {
            position = $scope.sequence.frames.length;
            $scope.sequence.frames.push([[], [], [], []])
        } else {
            $scope.sequence.frames.splice(position, 0, [[], [], [], []]);
        }

        frame = $scope.sequence.frames[position];

        for (var i = 0; i < 4; i++) {
            for (var j = 0; j < 25; j++) {
                if (based_on !== undefined) {
                    frame[i][j] = {color: based_on[i][j].color};
                } else {
                    frame[i][j] = {color: '#000000'};
                }
            }
        }
        $scope.select_frame(position);
    }

    $scope.remove_frame = function(position) {
        $scope.sequence.frames.splice(position, 1);
        if (position > 0) {
            position = position - 1;
        }
        $scope.select_frame(position);
    }

    $scope.select_frame = function(frame_id) {
        if (frame_id < 0) {
            frame_id = $scope.sequence.frames.length - 1;
        } else {
            frame_id = frame_id % $scope.sequence.frames.length;
        }
        $scope.leds = $scope.sequence.frames[frame_id];
        $scope.sequence.active_frame = frame_id;
    }

    $scope.set_percentage = function(percentage) {
        ac.percentage = Math.round(percentage);
        ac.slider.set(ac.percentage);
        val = 255 * (ac.percentage / 100);
        val = Math.round(val);
        hex = val.toString(16);
        if (hex.length == 1) {
            hex = '0' + hex;
        }
        ac.active_color = '#' + hex + hex + hex;
        $timeout(ac.percentage);
    }

    $scope.led_clicked = function(led, right=false) {
        if (right) {
            led.color = "#000000";
        } else {
            led.color = ac.active_color;
        }
    }

    $scope.multi_led_clicked = function(leds, right=false) {
        for (var i = 0; i < leds.length; i++) {
            $scope.led_clicked(leds[i], right);
        }
    }

    $scope.select_row = function(rownr, right=false) {
        for (var i = 0; i < $scope.leds[rownr].length; i++) {
            $scope.led_clicked($scope.leds[rownr][i], right);
        }
    }

    $scope.apply_all = function(right=false) {
        for (var i = 0; i < 4; i++) {
            for (var j = 0; j < 25; j++) {
                $scope.led_clicked($scope.leds[i][j], right);
            }
        }
    }

    $scope.toggle_simulate = function() {
        if ($scope.sequence.simulating) {
            $scope.sequence.simulating = false;
        } else {
            $scope.sequence.simulating = true;
            $scope.step();
        }
    }

    $scope.step = function() {
        if ($scope.sequence.simulating) {
            $scope.select_frame($scope.sequence.active_frame + 1);
            $timeout($scope.step, $scope.sequence.steptime);
        }
    }

    ac.exit = function() {
        window.close();
    }

    $scope.create_new_sequence(true);
});