$(document).ready(function() {
    $('.action-view-link').click(function() {
        var link = $(this).attr('rel');
        var overlay = $('div.dcloner-overlay');
        var oContents = '<div class="dclone-checker-container corners-4px">';

        oContents += '<div class="clone-checker-close"><a href="#close">Close</a></div>';
        oContents += '<iframe src="' + link + '" class="clone-checker" border="0"></iframe>';
        oContents += '</div>';
        

        if (overlay.length == 0) {
            $('body').prepend('<div class="dcloner-overlay"></div>');
        }
        
        $('.dcloner-overlay').html(oContents);

        $('.dclone-checker-container').Drags();

        $('a[href=#close]').click(function() {
            $('div.dcloner-overlay').remove();
        });

        return false;
    });
});
