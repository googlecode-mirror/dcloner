<div class="dcloner-summary">
    <div class="toggle-view">
        <button id="more-space" class="corners-4px" title="Need more space?">More space</button>
        <!--<button id="more-space-more">More space</button>-->
    </div>
    <ul id="container">
        <li class="dcloner-state statistics">
            <ul class="sublist">
                <li class="corners-4px shadow-all-4px">
                    <div class="dcloner-state-contents summary">
                        <span>
                        <?php print t('Active instances: !e', array('!e' => $stats['enabled'])); ?>
                        <br />
                        <?php print t('Disabled instances: !e', array('!e' => $stats['disabled'])); ?> /
                        <?php print t('Total: !e', array('!e' =>$stats['disabled'] + $stats['enabled'])); ?>
                        </span>
                    </div>
                </li>
                <li class="corners-4px shadow-all-4px">
                    <div class="dcloner-state-contents instances">
                        <span>
                            <?php print l(t('Instances'), 'admin/dcloner/short'); ?>
                        </span>
                    </div>
                </li>
                <li class="corners-4px shadow-all-4px">
                    <div class="dcloner-state-contents warnings">
                        <span>
                            <?php print l(t('Warnings ').$stat['warnings_cnt'], 'admin/dcloner/warns'); ?>
                        </span>
                    </div>
                </li>
                <li class="corners-4px shadow-all-4px">
                    <div class="dcloner-state-contents notification">
                        <span>
                            <?php print l(t('Notifications ').$stat['notifications_cnt'], 'admin/dcloner/notifications'); ?>
                        </span>
                    </div>
                </li>
            </ul>
        </li>
        <li class="dcloner-state active-container">
            <div class="active-content">
                <?php 
                    $url = preg_split('/\//', $_GET['q']);

                    if (!in_array('short', $url) || count($url == 2)) { 
                        print dcloner_list_instance(True);
                    }

                    if (in_array('warns', $url)) {
                        print $stat['warnings']; 
                    }
                    
                    if (in_array('notifications', $url)) {
                        print $stat['notifications']; 
                    }
                ?> 
            </div>
        </li>
    </ul>
</div>
