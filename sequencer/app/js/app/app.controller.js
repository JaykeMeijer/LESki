app.controller('AppController', function AppController($scope, $timeout, ngDialog) {
    var ac = this;
    ac.loaded = false;

    $scope.leds = [[], [] ,[] ,[]];
    ac.active_color = "#ff0000"

    function new_sequence() {
        $scope.sequence = {
            color: true,
            simulating: false,
            frames: [],
            active_frame: 0,
            steptime: 1000
        };
    }
    new_sequence();

    function init() {
        // Initialize the application
        var awaiting = 0;
        callback();

        function callback() {
            awaiting--;
            if (awaiting < 1) {
                ac.loaded = true;
            }
        }
    }
    init();

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
                    if ($scope.sequence.color) {
                        val = htmlToRGB(row[k].color)
                        writeByte(val.r);
                        writeByte(val.g);
                        writeByte(val.b);
                    } else {
                        writeByte(1);
                    }
                    
                }
                writeByte(11);
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

        console.log($scope.sequence);

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
                        row[ledpointer] = b;
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
    $scope.create_new_frame(0);

    $scope.led_clicked = function(led) {
        led.color = ac.active_color;
    }

    $scope.apply_all = function(color) {
        for (var i = 0; i < 4; i++) {
            for (var j = 0; j < 25; j++) {
                $scope.leds[i][j].color = color;
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

});