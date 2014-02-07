<?php
/**
 * @file
 * todo: write file info
 */

/**
 * USING COMMERCE PAYMENT API
 */

/**
 * Builds the onlymoney payment method settings
 */
function commerce_onlymoney_settings_form($settings = NULL) {
  $form = array();

  if (empty($settings['commerce_onlymoney_secret_key'])) {
    drupal_set_message(t(check_plain('Please, visit !link and fill the form'),
                         array(
                           '!link' => l(t('https://onlymoney.com/merchant/'),
                                        'https://onlymoney.com/merchant/',
                                        array('absolute' => TRUE))
                         )), 'error');

    $settings['commerce_onlymoney_secret_key'] = '';
    $settings['commerce_onlymoney_merchant_id'] = '';
    $settings['commerce_onlymoney_api_url'] = 'https://onlymoney.com/customs/';
  }

  $form['commerce_onlymoney_secret_key'] = array(
    '#title' => t('Secret Key'),
    '#type' => 'textfield',
    '#description' => t('See !link', array(
      '!link' => l(t('https://onlymoney.com/merchant/'),
                   'https://onlymoney.com/merchant/', array(
          'absolute' => TRUE,
          'attributes' => array('target' => '_blank')
        ))
    )),
    '#size' => 46,
    '#required' => TRUE,
    '#default_value' => $settings['commerce_onlymoney_secret_key'],
  );

  $form['commerce_onlymoney_merchant_id'] = array(
    '#title' => t('Merchant ID'),
    '#type' => 'textfield',
    '#descrtiption' => t(''),
    '#size' => 8,
    '#required' => TRUE,
    '#default_value' => $settings['commerce_onlymoney_merchant_id'],
  );

  $form['commerce_onlymoney_api_url'] = array(
    '#title' => t('Onlymoney API URL'),
    '#type' => 'textfield',
    '#descrtiption' => t(''),
    '#size' => 26,
    '#required' => TRUE,
    '#default_value' => $settings['commerce_onlymoney_api_url'],
    '#disabled' => TRUE,
  );

  $form['commerce_onlymoney_cancel_url'] = array(
    '#title' => t('Onlymoney Cancel Page URL'),
    '#type' => 'textfield',
    '#size' => 26,
    '#required' => FALSE,
    '#field_prefix' =>
      url('onlymoney/cancel', array('absolute' => TRUE)) . ' --&gt; ',
    '#default_value' => $settings['commerce_onlymoney_cancel_url'],
    '#description' => 'You can ented the page where user will be redirected to after cancelling the payment <br> You can use alias URL or direct native path: <b>node/1</b> or <b>taxonomy/term/1</b>',
  );

  $form['commerce_onlymoney_logging_mode'] = array(
    '#title' => t('Onlymoney Logging Mode'),
    '#type' => 'checkbox',
    '#default_value' => $settings['commerce_onlymoney_logging_mode'],
    '#description' => t('If checked, module will write logs with watchdog while payment process'),
  );

  return $form;
}
