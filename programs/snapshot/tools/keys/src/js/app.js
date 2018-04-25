const $         = require('jquery'),
      tabby     = require('./tabby.js'),
      util      = require('./utilities.js'),
      clipboard = require('clipboard')

const app     = {},
      helpers = {},
      gui     = {},
      bind    = {}

let   entropied = false;

$(() => {
  bind.tabby()
  bind.reset()
  bind.regenerate()
  gui.reset()
  window.addEventListener('will-navigate', e => e.preventDefault());

  new clipboard('#keyGen input', {
    target: function(trigger) {
        return trigger;
    }
  });

})

app.validator = (sel) => {
  $(sel).find('form').submit( function(e){
    e.preventDefault()
    const $fields = $(this).find('input'),
          $pub    = $(this).find('input[name=public_key]'),
          $priv   = $(this).find('input[name=private_key]'),
          pub     = $pub.val(),
          priv    = $priv.val(),
          isValid = util.validateKeypair(pub, priv)

    $('body').removeClass('error valid')

    if(isValid)
      $('body').addClass('valid')
    else
      $('body').addClass('error')
  })
}

app.keyGen = (sel) => {
  if(!entropied) $('body').addClass('entropy')
  util.genKeyPair().then( keypair => {
    gui.displayKey( keypair )
    $('body').removeClass('entropy')
    entropied = true
  })
}

//GUI
gui.displayKey = keypair => {
  const $form = $('#keyGen form')
  $form.find('input[name=public_key]').attr('value', keypair.public)
  $form.find('input[name=private_key]').attr('value', keypair.private)
}

gui.reset = () => {
  gui.bodyID('welcome')
  $('body').removeClass('error valid')

  const $output = $('#keyGen form')
  $output.find('input[name=public_key]').attr('value', null)
  $output.find('input[name=private_key]').attr('value', null)

  const $validator = $('#validator form')
  $validator.find('input[name=public_key]').attr('value', null)
  $validator.find('input[name=private_key]').attr('value', null)

  tabby.toggleTab('#welcome')
}

gui.validatorReady = ( form ) = {
  // if(form)
}

gui.bodyID = id => {
    $('body').attr('id', `${id}-active`)
}

//Bindings
bind.reset = () => {
  $('a.reset').on('click', gui.reset)
}

bind.regenerate = () => {
  $('a.regenerate').on('click', app.keyGen)
}

bind.tabby = () => {
  tabby.init({
    callback: initApp
  })
}

const initApp = (tabs, toggle) => {
  const id = $(tabs).attr('id')
  if(typeof app[id] === 'function')
    gui.bodyID(id),
    app[id](`#${id}`)
    // typography(`#${id}`)
  return true
}

const typography = (sel) => {
  flowtype(document.querySelector(sel), {
    minimum   : 500,
    maximum   : 1200,
    minFont   : 12,
    maxFont   : 40,
    fontRatio : 30
  });
}
