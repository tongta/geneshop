<?php
/**
 * @file
 * Page callbacks for module main settings.
 */
function geneshop_admin_settings($form, &$form_state) {
  $form['#prefix'] = '<div id="geneshop_settings_form-wrapper">';
  $form['#suffix'] = '</div>';
  $form['general'] = array(
    '#type' => 'fieldset',
    '#title' => t('General settings'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );
  $form['general']['geneshop_basket_email'] = array(
    '#type' => 'textfield',
    '#title' => t('E-mail'),
    '#required' => TRUE,
    '#default_value' => variable_get('geneshop_basket_email', variable_get('site_mail', ini_get('sendmail_from'))),
    '#description' => t('E-mail address to which the order will be sent.'),
  );
  $form['general']['geneshop_order_title'] = array(
    '#type' => 'textfield',
    '#title' => t('Order title'),
    '#required' => TRUE,
    '#default_value' => variable_get('geneshop_order_title', t('Order #[node:order-id] | [node:created]')),
    '#description' => t('This string will be used as title for orders. Use the syntax [token] if you want to insert a replacement pattern.'),
  );
  $form['general']['title_tokens'] = array(
    '#type' => 'fieldset',
    '#title' => t('Replacement patterns'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $info = token_info();
  $tokens = array();
  foreach ($info['tokens']['node'] as $name => $token) {
    if ($name == 'title') {
      continue;
    }
    $tokens[] = '[node:' . $name . '] - ' . $token['description'];
  }
  $form['general']['title_tokens']['list'] = array(
    '#markup' => implode('<br />', $tokens),
  );
  $options = node_type_get_names();
  unset($options[geneshop_ORDER_NODETYPE]);
  $form['general']['geneshop_node_types'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Content types for Gene Shop'),
    '#options' => $options,
    '#default_value' => variable_get('geneshop_node_types', array()),
    '#ajax' => array(
      'event' => 'change',
      'callback' => 'geneshop_settings_form_ajax_callback',
      'wrapper' => 'geneshop_settings_form-wrapper',
    ),
  );
  $basket_types = isset($form_state['values']['geneshop_node_types']) ? array_keys(array_filter($form_state['values']['geneshop_node_types'])) : variable_get('geneshop_node_types', array());
  if ($basket_types) {
    $form['display_options'] = array(
      '#type' => 'fieldset',
      '#title' => t('Display settings'),
      '#collapsible' => FALSE,
      '#collapsed' => FALSE,
    );
    $form['display_options']['geneshop_show_qty'] = array(
      '#type' => 'checkbox',
      '#title' => t('Display editable field with the quantity of selected items.'),
      '#default_value' => variable_get('geneshop_show_qty', TRUE),
    );
    if (module_exists('number')) {
      foreach ($basket_types as $type) {
        $fields = field_info_instances('node', $type);
        foreach ($fields as $field => $data) {
          if ($data['widget']['type'] == 'number') {
            $field_list[$field] = $data['label'] . ' (' . $field . ')';
          }
        }
      }
      if (!empty($field_list)) {
        $form['display_options']['geneshop_show_price'] = array(
          '#type' => 'checkbox',
          '#title' => t('Display price data for basket items.'),
          '#default_value' => variable_get('geneshop_show_price', FALSE),
        );
        array_unshift($field_list, '');
        $form['price_options'] = array(
          '#type' => 'fieldset',
          '#title' => t('Fields setting'),
          '#collapsible' => FALSE,
          '#collapsed' => FALSE,
          '#states' => array(
            'visible' => array(
              ':input[name="geneshop_show_price"]' => array('checked' => TRUE),
            ),
          ),
        );
        $form['price_options']['geneshop_price_field'] = array(
          '#type' => 'select',
          '#title' => t('Select a price field'),
          '#options' => $field_list,
          '#default_value' => variable_get('geneshop_price_field', ''),
          '#description' => t('The field(integer or float types) whose value will be the price of the product. Option is disabled if no fields selected.'),
        );
        $form['price_options']['geneshop_price_suffix'] = array(
          '#type' => 'textfield',
          '#title' => t('Suffix for price (currency)'),
          '#default_value' => variable_get('geneshop_price_suffix', ''),
          '#size' => 20,
          '#maxlength' => 20,
          '#description' => t('Suffix to be added to the value of the price. Eg: $, rub, euro'),
        );
      }
    }
  }
  $form['array_filter'] = array(
    '#type' => 'value',
    '#value' => TRUE,
  );
  $form['#submit'] = array('drupal_flush_all_caches');
  return system_settings_form($form);
}
function geneshop_settings_form_ajax_callback($form, &$form_state) {
  return $form;
}
function geneshop_admin_settings_validate($form, &$form_state) {
  $values = &$form_state['values'];
  $types = array_filter($values['geneshop_node_types']);
  if (!$types) {
    $values['geneshop_show_price'] = FALSE;
    $values['geneshop_price_field'] = '';
    $values['geneshop_price_suffix'] = '';
    return;
  }
  if (isset($values['geneshop_show_price']) && $values['geneshop_show_price'] == TRUE) {
    if (!empty($values['geneshop_price_field'])) {
      foreach ($types as $type) {
        if (!field_info_instance('node', $values['geneshop_price_field'], $type)) {
          form_set_error('geneshop_price_field', t('The selected field %geneshop_price_field is not enabled in the node type %type', array('%geneshop_price_field' => $values['geneshop_price_field'], '%type' => $type)));
        }
      }
    }
  }
  $mail_fields = variable_get('geneshop_mail_fields', array());
  if (empty($mail_fields)) {
    $form_state['redirect'] = 'admin/config/geneshop/mail';
  }
}
function geneshop_admin_mail_settings($form, &$form_state) {
  $form['#prefix'] = t('Showing only those fields that can be displayed in plain text in emails.');
  $form['user'] = array(
    '#type' => 'fieldset',
    '#title' => t('Email notification settings for user'),
  );
  $form['user']['geneshop_mail_user_send'] = array(
    '#type' => 'checkbox',
    '#title' => t('Send order data on user mail.'),
    '#default_value' => variable_get('geneshop_mail_user_send', FALSE),
  );
  if ($fields = field_info_instances('node', geneshop_ORDER_NODETYPE)) {
    $fields_list = array();
    // Only those fields that can be displayed in plain text in emails.
    $field_modules_filter = array('number', 'text', 'list', 'options');
    foreach ($fields as $name => $field) {
      if (in_array($field['display']['default']['module'], $field_modules_filter)) {
        $fields_list[$name] = $field['label'] . ' (' . $name . ')';
      }
    }
    $form['site'] = array(
      '#type' => 'fieldset',
      '#title' => t('Email notification settings for site'),
    );
    $form['site']['geneshop_mail_fields_site'] = array(
      '#type' => 'checkboxes',
      '#options' => $fields_list,
      '#title' => t('These fields, which will be sent to the email address of the site'),
      '#default_value' => variable_get('geneshop_mail_fields_site', array()),
    );
    $form['user']['geneshop_mail_fields_user'] = array(
      '#type' => 'checkboxes',
      '#options' => $fields_list,
      '#title' => t('These fields, which will be sent to the email address of the user'),
      '#default_value' => variable_get('geneshop_mail_fields_user', array()),
      '#states' => array(
        'visible' => array(
          ':input[name="geneshop_mail_user_send"]' => array('checked' => TRUE),
        ),
      ),
    );
    $form['array_filter'] = array(
      '#type' => 'value',
      '#value' => TRUE,
    );
  }
  return system_settings_form($form);
}
