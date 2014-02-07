<?php
/**
 * @file
 * Описание файла
 */

/**
 * Menu Callback: onlymoney/cancel
 *
 * @return drupal output | redirect 301
 * @see commerce_onlymoney_menu
 */
function commerce_onlymoney_page_cancel() {
  $settings = _commerce_onlymoney_get_settings();
  if (!empty($settings['commerce_onlymoney_cancel_url'])) {
    drupal_goto($settings['commerce_onlymoney_cancel_url'], array(), 301);
  }

  $output = t('Cancel');

  return $output;
}

/**
 * Menu Callback: onlymoney/status
 *
 * @return string
 * @see commerce_onlymoney_menu
 */
function commerce_onlymoney_page_status() {

  $settings = _commerce_onlymoney_get_settings();
  $watchdog_mode = (bool) $settings['commerce_onlymoney_logging_mode'];

  if (!empty($_POST['signature']) && !empty($_POST['operation'])) {
    $operation = json_decode(urldecode(base64_decode($_POST['operation'])));

    /**
     * Find Order
     */
    $order = commerce_order_load($operation->order_id);
    if (empty($order)) {
      if ($watchdog_mode) {
        watchdog(WATCHDOG_ERROR, 'wrong payment order');
      }
      print json_encode(array(
                          'err' => 1,
                          'message' => t('wrong payment order')
                        ));
      exit();
    }

    if (!empty($order->commerce_order_total)) {
      if (!empty($order->commerce_order_total[LANGUAGE_NONE][0]['amount'])) {
        $amount = $order->commerce_order_total[LANGUAGE_NONE][0]['amount'];
      }

      if (!empty($order->commerce_order_total[LANGUAGE_NONE][0]['currency_code'])) {
        $currency_code = $order->commerce_order_total[LANGUAGE_NONE][0]['currency_code'];
      }
    }
    else {
      if ($watchdog_mode) {
        watchdog(WATCHDOG_ERROR, 'wrong payment order on merchant site');
      }
      print json_encode(array(
                          'err' => 1,
                          'message' => t('wrong payment order on merchant site')
                        ));
      exit();
    }

    if ($amount / 100 != $operation->amount) {
      if ($watchdog_mode) {
        watchdog(WATCHDOG_ERROR,
                 'wrong payment amount (our ' . $amount / 100 . ' != '
                 . $operation->amount . ')');
      }
      print json_encode(array(
                          'err' => 1,
                          'message' => t('wrong payment amount')
                        ));
      exit();
    }

    if ($currency_code != $operation->currency) {
      if ($watchdog_mode) {
        watchdog(WATCHDOG_ERROR, 'wrong payment currency');
      }
      print json_encode(array(
                          'err' => 1,
                          'message' => t('wrong payment currency')
                        ));
      exit();
    }


    /**
     * Find Signature
     */
    $secret_key = $settings['commerce_onlymoney_secret_key'];

    $json = urldecode(base64_decode($_POST['operation']));
    $merchant_signature = sha1($secret_key . $json . $secret_key);
    $signature = urldecode(base64_decode($_POST['signature']));
    if ($merchant_signature != $signature) {
      if ($watchdog_mode) {
        watchdog(WATCHDOG_ERROR,
                 'wrong signature (NEW: ' . $merchant_signature . ' SERVER: '
                 . $signature . ')');
      }
      print json_encode(array(
                          'err' => 1,
                          'message' => t('wrong signature')
                        ));
      exit();
    }

  }

  else {
    if ($watchdog_mode) {
      watchdog(WATCHDOG_ERROR, 'wrong signature or order');
    }
    print json_encode(array(
                        'err' => 1,
                        'message' => t('wrong signature or order')
                      ));
    exit();
  }

  /**
   * return json encoded string
   */
  if ($watchdog_mode) {
    watchdog(WATCHDOG_INFO, 'payment ok');
  }
  print json_encode(array('err' => 0, 'message' => 'payment ok'));
  exit();
}

/**
 * Menu Callback: onlymoney/success
 *
 * authenticate input POST request
 * and close opened transaction
 *
 * @see commerce_onlymoney_menu
 */
function commerce_onlymoney_page_success() {

  $output = '';
  $settings = _commerce_onlymoney_get_settings();
  $watchdog_mode = (bool) $settings['commerce_onlymoney_logging_mode'];

  if (!empty($_POST['order_id']) && !empty($_POST['amount'])
    && is_object($order = commerce_order_load($_POST['order_id']))
  ) {
    if (!empty($order->commerce_order_total)) {
      if (!empty($order->commerce_order_total[LANGUAGE_NONE][0]['amount'])) {
        $amount = $order->commerce_order_total[LANGUAGE_NONE][0]['amount'];
      }

      if (!empty($order->commerce_order_total[LANGUAGE_NONE][0]['currency_code'])) {
        $currency = $order->commerce_order_total[LANGUAGE_NONE][0]['currency_code'];
      }
    }

    if (!empty($amount) && !empty($currency)
      && $amount / 100 == $_POST['amount']
      && $currency == $_POST['currency']
    ) {

      /**
       * save transaction
       */
      $transaction_id = db_select('commerce_payment_transaction', 't')
        ->fields('t', array('transaction_id', 'order_id'))
        ->condition('t.order_id', $order->order_number)->range(0, 1)
        ->orderBy('transaction_id', 'DESC')->execute()->fetchField();

      if (!empty($transaction_id)) {
        $transaction = commerce_payment_transaction_load($transaction_id);
        $transaction->status = COMMERCE_PAYMENT_STATUS_SUCCESS;
        commerce_payment_transaction_save($transaction);

        // update order to payed
        $loaded_order = commerce_order_load($order->order_number);
        $loaded_order->status = 'processing';
        commerce_order_save($loaded_order);

        rules_invoke_all('commerce_checkout_complete', $order);
      }
      else {
        if ($watchdog_mode) {
          watchdog(WATCHDOG_ERROR, 'Wrong site transaction');
        }
        drupal_set_message(t('Wrong site transaction'), 'error');
      }

    }
    else {
      if ($watchdog_mode) {
        watchdog(WATCHDOG_ERROR, 'Wrong amount or currency code');
      }
      drupal_set_message(t('Wrong amount or currency code'), 'error');
    }

    $output = t('Thank you for purchasing. Our manager will contact you shortly');
    $output .= render(commerce_order_ui_order_view($order, 'customer'));
  }
  else {
    if ($watchdog_mode) {
      watchdog(WATCHDOG_ERROR, 'Wrong order data');
    }
    drupal_set_message(t('Wrong order data'), 'error');
  }

  return $output;
}

/**
 * Menu Callback: onlymoney/fail
 *
 * @return drupal output
 * @see commerce_onlymoney_menu
 */
function commerce_onlymoney_page_failed() {
  $settings = _commerce_onlymoney_get_settings();
  if ((bool) $settings['commerce_onlymoney_logging_mode']) {
    watchdog(WATCHDOG_ERROR, 'Onlymoney payment pay fail: '.serialize($_POST));
  }

  $output = t('Sorry, payment failed. Please, contact site administration');
  return $output;
}
