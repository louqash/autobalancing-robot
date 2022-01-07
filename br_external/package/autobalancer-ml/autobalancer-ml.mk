################################################################################
#
# autobalancer_ml
#
################################################################################

AUTOBALANCER_ML_VERSION = 0.2
AUTOBALANCER_ML_SITE = $(BR2_EXTERNAL)/src/ml
AUTOBALANCER_ML_SITE_METHOD = local

$(eval $(cmake-package))
