$(document).ready(function() {
    var toggle_state = false;

    $('#more-space').click(function() {        
        if (!toggle_state) {
            $('.statistics').animate({width: 'hide'});
        } else {
            $('.statistics').animate({width: 'show'});
        }

        toggle_state = !toggle_state;
    });
});
