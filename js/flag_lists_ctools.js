/**
* Provide the HTML to create the modal dialog.
*/
Drupal.theme.prototype.FlagListsModal = function () {
  var html = '';

  html += '<div id="ctools-modal" class="flag-lists-modal">';
  html += '  <div class="ctools-modal-content flag-lists-modal">';

  html += '          <div class="popups-container">';
  html += '            <div class="modal-header popups-title">';
  html += '              <span id="modal-title" class="modal-title"></span>';
  html += '              <span class="popups-close"><a class="close" href="#">' + Drupal.CTools.Modal.currentSettings.closeText + '</a></span>';
  html += '              <div class="clear-block"></div>';
  html += '            </div>';
  html += '            <div class="modal-scroll"><div id="modal-content" class="modal-content popups-body"></div></div>';
  html += '            <div class="popups-buttons"></div>'; //Maybe someday add the option for some specific buttons.
  html += '            <div class="popups-footer"></div>'; //Maybe someday add some footer.
  html += '          </div>';

  html += '  </div>';
  html += '</div>';

  return html;

};
