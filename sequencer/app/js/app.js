// Define the `phonecatApp` module
var app = angular.module('app', ['ngDialog']);

app
    .directive('autofocus', ['$timeout', function($timeout) {
        return {
            restrict: 'A',
            link : function($scope, $element) {
                $timeout(function() {
                    $element[0].focus();
                });
            }
        }
    }]);